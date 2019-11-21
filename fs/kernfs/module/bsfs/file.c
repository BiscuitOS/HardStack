/*
 * Bsfs file.c
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/seq_file.h>

#include "linux/bsfs.h"

static DEFINE_SPINLOCK(bsfs_open_node_lock);
static DEFINE_MUTEX(bsfs_open_file_mutex);

static void bsfs_release_file(struct bsfs_node *kn,
				struct bsfs_open_file *of)
{
	lockdep_assert_held(&bsfs_open_file_mutex);

	if (!of->released) {
		kn->attr.ops->release(of);
		of->released = true;
	}
}

static struct bsfs_open_file *bsfs_of(struct file *file)
{
	return ((struct seq_file *)file->private_data)->private;
}

static void bsfs_put_open_node(struct bsfs_node *kn,
				struct bsfs_open_file *of)
{
	struct bsfs_open_node *on = kn->attr.open;
	unsigned long flags;

	mutex_lock(&bsfs_open_file_mutex);
	spin_lock_irqsave(&bsfs_open_node_lock, flags);

	if (of)
		list_del(&of->list);

	if (atomic_dec_and_test(&on->refcnt))
		kn->attr.open = NULL;
	else
		on = NULL;

	spin_unlock_irqrestore(&bsfs_open_node_lock, flags);
	mutex_unlock(&bsfs_open_file_mutex);

	kfree(on);
}

void bsfs_drain_open_files(struct bsfs_node *kn)
{
	struct bsfs_open_node *on;
	struct bsfs_open_file *of;

	if (!(kn->flags & (BSFS_HAS_MMAP | BSFS_HAS_RELEASE)))
		return;

	spin_lock_irq(&bsfs_open_node_lock);
	on = kn->attr.open;
	if (on)
		atomic_inc(&on->refcnt);
	spin_unlock_irq(&bsfs_open_node_lock);
	if (!on)
		return;

	mutex_lock(&bsfs_open_file_mutex);

	list_for_each_entry(of, &on->files, list) {
		struct inode *inode = file_inode(of->file);

		if (kn->flags & BSFS_HAS_MMAP)
			unmap_mapping_range(inode->i_mapping, 0, 0, 1);

		if (kn->flags & BSFS_HAS_RELEASE)
			bsfs_release_file(kn, of);
	}

	mutex_unlock(&bsfs_open_file_mutex);

	bsfs_put_open_node(kn, NULL);
}

static const struct bsfs_ops *bsfs_ops(struct bsfs_node *kn)
{
	if (kn->flags & BSFS_LOCKDEP)
		lockdep_assert_held(kn);
	return kn->attr.ops;
}

static ssize_t bsfs_file_direct_read(struct bsfs_open_file *of,
			char __user *user_buf, size_t count,
			loff_t *ppos)
{
	ssize_t len = min_t(size_t, count, PAGE_SIZE);
	const struct bsfs_ops *ops;
	char *buf;

	buf = of->prealloc_buf;
	if (buf)
		mutex_lock(&of->prealloc_mutex);
	else
		buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	mutex_lock(&of->mutex);
	if (!bsfs_get_active(of->kn)) {
		len = -ENODEV;
		mutex_unlock(&of->mutex);
		goto out_free;
	}

	of->event = atomic_read(&of->kn->attr.open->event);
	ops = bsfs_ops(of->kn);
	if (ops->read)
		len = ops->read(of, buf, len, *ppos);
	else
		len = -EINVAL;

	bsfs_put_active(of->kn);
	mutex_unlock(&of->mutex);

	if (len < 0)
		goto out_free;
	if (copy_to_user(user_buf, buf, len)) {
		len = -EFAULT;
		goto out_free;
	}
	*ppos += len;

out_free:
	if (buf == of->prealloc_buf)
		mutex_unlock(&of->prealloc_mutex);
	else
		kfree(buf);
	return len;
}

static ssize_t bsfs_fop_read(struct file *file, char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct bsfs_open_file *of = bsfs_of(file);

	if (of->kn->flags & BSFS_HAS_SEQ_SHOW)
		return seq_read(file, user_buf, count, ppos);
	else
		return bsfs_file_direct_read(of, user_buf, count, ppos);
}

static void bsfs_vma_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct bsfs_open_file *of = bsfs_of(file);

	if (!of->vm_ops)
		return;

	if (!bsfs_get_active(of->kn))
		return;

	if (of->vm_ops->open)
		of->vm_ops->open(vma);

	bsfs_put_active(of->kn);
}

static vm_fault_t bsfs_vma_fault(struct vm_fault *vmf)
{
	struct file *file = vmf->vma->vm_file;
	struct bsfs_open_file *of = bsfs_of(file);
	vm_fault_t ret;

	if (!of->vm_ops)
		return VM_FAULT_SIGBUS;

	if (!bsfs_get_active(of->kn))
		return VM_FAULT_SIGBUS;

	ret = VM_FAULT_SIGBUS;
	if (of->vm_ops->fault)
		ret = of->vm_ops->fault(vmf);

	bsfs_put_active(of->kn);
	return ret;
}

static vm_fault_t bsfs_vma_page_mkwrite(struct vm_fault *vmf)
{
	struct file *file = vmf->vma->vm_file;
	struct bsfs_open_file *of = bsfs_of(file);
	vm_fault_t ret;

	if (!of->vm_ops)
		return VM_FAULT_SIGBUS;

	if (!bsfs_get_active(of->kn))
		return VM_FAULT_SIGBUS;

	ret = 0;
	if (of->vm_ops->page_mkwrite)
		ret = of->vm_ops->page_mkwrite(vmf);
	else
		file_update_time(file);

	bsfs_put_active(of->kn);
	return ret;
}

static int bsfs_vma_access(struct vm_area_struct *vma, unsigned long addr,
			void *buf, int len, int write)
{
	struct file *file = vma->vm_file;
	struct bsfs_open_file *of = bsfs_of(file);
	int ret;

	if (!of->vm_ops)
		return -EINVAL;

	if (!bsfs_get_active(of->kn))
		return -EINVAL;

	ret = -EINVAL;
	if (of->vm_ops->access)
		ret = of->vm_ops->access(vma, addr, buf, len, write);

	bsfs_put_active(of->kn);
	return ret;
}

static const struct vm_operations_struct bsfs_vm_ops = {
	.open		= bsfs_vma_open,
	.fault		= bsfs_vma_fault,
	.page_mkwrite	= bsfs_vma_page_mkwrite,
	.access		= bsfs_vma_access,
};

static int bsfs_fop_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct bsfs_open_file *of = bsfs_of(file);
	const struct bsfs_ops *pos;
	int rc;

	if (!(of->kn->flags & BSFS_HAS_MMAP))
		return -ENODEV;

	mutex_lock(&of->mutex);

	rc = -ENODEV;
	if (!bsfs_get_active(of->kn))
		goto out_unlock;

	pos = bsfs_ops(of->kn);
	rc = pos->mmap(of, vma);
	if (rc)
		goto out_put;

	if (vma->vm_file != file)
		goto out_put;

	rc = -EINVAL;
	if (of->mmapped && of->vm_ops != vma->vm_ops)
		goto out_put;

	rc = -EINVAL;
	if (vma->vm_ops && vma->vm_ops->close)
		goto out_put;

	rc = 0;
	of->mmapped = true;
	of->vm_ops = vma->vm_ops;
	vma->vm_ops = &bsfs_vm_ops;
out_put:
	bsfs_put_active(of->kn);
out_unlock:
	mutex_unlock(&of->mutex);

	return rc;
}

static int bsfs_fop_release(struct inode *inode, struct file *filp)
{
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_open_file *of = bsfs_of(filp);

	if (kn->flags & BSFS_HAS_RELEASE) {
		mutex_lock(&bsfs_open_file_mutex);
		bsfs_release_file(kn, of);
		mutex_unlock(&bsfs_open_file_mutex);
	}

	bsfs_put_open_node(kn, of);
	seq_release(inode, filp);
	kfree(of->prealloc_buf);
	kfree(of);

	return 0;
}

static __poll_t bsfs_fop_poll(struct file *filp, poll_table *wait)
{
	struct bsfs_open_file *of = bsfs_of(filp);
	struct bsfs_node *kn = bsfs_dentry_node(filp->f_path.dentry);
	struct bsfs_open_node *on = kn->attr.open;

	if (!bsfs_get_active(kn))
		goto trigger;

	poll_wait(filp, &on->poll, wait);

	bsfs_put_active(kn);

	if (of->event != atomic_read(&on->event))
		goto trigger;

	return DEFAULT_POLLMASK;

trigger:
	return DEFAULT_POLLMASK | EPOLLERR | EPOLLPRI;
}

static int bsfs_get_open_node(struct bsfs_node *kn,
				struct bsfs_open_file *of)
{
	struct bsfs_open_node *on, *new_on = NULL;

retry:
	mutex_lock(&bsfs_open_file_mutex);
	spin_lock_irq(&bsfs_open_node_lock);

	if (!kn->attr.open && new_on) {
		kn->attr.open = new_on;
		new_on = NULL;
	}

	on = kn->attr.open;
	if (on) {
		atomic_inc(&on->refcnt);
		list_add_tail(&of->list, &on->files);
	}

	spin_unlock_irq(&bsfs_open_node_lock);
	mutex_unlock(&bsfs_open_file_mutex);

	if (on) {
		kfree(new_on);
		return 0;
	}

	new_on = kmalloc(sizeof(*new_on), GFP_KERNEL);
	if (!new_on)
		return -ENOMEM;

	atomic_set(&new_on->refcnt, 0);
	atomic_set(&new_on->event, 1);
	init_waitqueue_head(&new_on->poll);
	INIT_LIST_HEAD(&new_on->files);
	goto retry;
}

static void bsfs_seq_stop_active(struct seq_file *sf, void *v)
{
	struct bsfs_open_file *of = sf->private;
	const struct bsfs_ops *ops = bsfs_ops(of->kn);

	if (ops->seq_stop)
		ops->seq_stop(sf, v);
	bsfs_put_active(of->kn);
}

static void *bsfs_seq_start(struct seq_file *sf, loff_t *ppos)
{
	struct bsfs_open_file *of = sf->private;
	const struct bsfs_ops *ops;

	mutex_lock(&of->mutex);
	if (!bsfs_get_active(of->kn))
		return ERR_PTR(-ENODEV);

	ops = bsfs_ops(of->kn);
	if (ops->seq_start) {
		void *next = ops->seq_start(sf, ppos);
		if (next == ERR_PTR(-ENODEV))
			bsfs_seq_stop_active(sf, next);
		return next;
	} else {
		return NULL + !*ppos;
	}
}

static void *bsfs_seq_next(struct seq_file *sf, void *v, loff_t *ppos)
{
	struct bsfs_open_file *of = sf->private;
	const struct bsfs_ops *ops = bsfs_ops(of->kn);

	if (ops->seq_next) {
		void *next = ops->seq_next(sf, v, ppos);
		if (next == ERR_PTR(-ENODEV))
			bsfs_seq_stop_active(sf, next);
		return next;
	} else {
		++*ppos;
		return NULL;
	}
}

static void bsfs_seq_stop(struct seq_file *sf, void *v)
{
	struct bsfs_open_file *of = sf->private;

	if (v != ERR_PTR(-ENODEV))
		bsfs_seq_stop_active(sf, v);
	mutex_unlock(&of->mutex);
}

static int bsfs_seq_show(struct seq_file *sf, void *v)
{
	struct bsfs_open_file *of = sf->private;

	of->event = atomic_read(&of->kn->attr.open->event);

	return of->kn->attr.ops->seq_show(sf, v);
}

static const struct seq_operations bsfs_seq_ops = {
	.start	= bsfs_seq_start,
	.next	= bsfs_seq_next,
	.stop	= bsfs_seq_stop,
	.show	= bsfs_seq_show,
};

static int bsfs_fop_open(struct inode *inode, struct file *file)
{
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_root *root = bsfs_root(kn);
	const struct bsfs_ops *ops;
	struct bsfs_open_file *of;
	bool has_read, has_write, has_mmap;
	int error = -EACCES;

	if (!bsfs_get_active(kn))
		return -ENODEV;

	ops = bsfs_ops(kn);

	has_read = ops->seq_show || ops->read || ops->mmap;
	has_write = ops->write || ops->mmap;
	has_mmap = ops->mmap;

	if (root->flags & BSFS_ROOT_EXTRA_OPEN_PERM_CHECK) {
		if ((file->f_mode & FMODE_WRITE) &&
			(!(inode->i_mode & S_IWUGO) || !has_write))
			goto err_out;

		if ((file->f_mode & FMODE_READ) &&
			(!(inode->i_mode & S_IRUGO) || !has_read))
			goto err_out;
	}
	error = -ENOMEM;
	of = kzalloc(sizeof(struct bsfs_open_file), GFP_KERNEL);
	if (!of)
		goto err_out;

	if (has_mmap)
		mutex_init(&of->mutex);
	else
		mutex_init(&of->mutex);

	of->kn = kn;
	of->file = file;

	of->atomic_write_len = ops->atomic_write_len;
	error = -EINVAL;

	if (ops->prealloc && ops->seq_show)
		goto err_free;
	if (ops->prealloc) {
		int len = of->atomic_write_len ? : PAGE_SIZE;
		of->prealloc_buf = kmalloc(len + 1, GFP_KERNEL);
		error = -ENOMEM;
		if (!of->prealloc_buf)
			goto err_free;
		mutex_init(&of->prealloc_mutex);
	}

	if (ops->seq_show)
		error = seq_open(file, &bsfs_seq_ops);
	else
		error = seq_open(file, NULL);
	if (error)
		goto err_free;

	of->seq_file = file->private_data;
	of->seq_file->private = of;

	if (file->f_mode & FMODE_WRITE)
		file->f_mode |= FMODE_PWRITE;

	error = bsfs_get_open_node(kn, of);
	if (error)
		goto err_seq_release;

	if (ops->open) {
		error = ops->open(of);
		if (error)
			goto err_put_node;
	}

	bsfs_put_active(kn);
	return 0;

err_put_node:
	bsfs_put_open_node(kn, of);
err_seq_release:
	seq_release(inode, file);
err_free:
	kfree(of->prealloc_buf);
	kfree(of);
err_out:
	bsfs_put_active(kn);
	return error;
}

static ssize_t bsfs_fop_write(struct file *file, const char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct bsfs_open_file *of = bsfs_of(file);
	const struct bsfs_ops *ops;
	ssize_t len;
	char *buf;

	if (of->atomic_write_len) {
		len = count;
		if (len > of->atomic_write_len)
			return -E2BIG;
	} else {
		len = min_t(size_t, count, PAGE_SIZE);
	}

	buf = of->prealloc_buf;
	if (buf)
		mutex_lock(&of->prealloc_mutex);
	else
		buf = kmalloc(len + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, user_buf, len)) {
		len = -EFAULT;
		goto out_free;
	}
	buf[len] = '\0';

	mutex_lock(&of->mutex);
	if (!bsfs_get_active(of->kn)) {
		mutex_unlock(&of->mutex);
		len = -ENODEV;
		goto out_free;
	}

	ops = bsfs_ops(of->kn);
	if (ops->write)
		len = ops->write(of, buf, len, *ppos);
	else
		len = -EINVAL;

	bsfs_put_active(of->kn);
	mutex_unlock(&of->mutex);

	if (len > 0)
		*ppos += len;

out_free:
	if (buf == of->prealloc_buf)
		mutex_unlock(&of->prealloc_mutex);
	else
		kfree(buf);
	return len;
}

const struct file_operations bsfs_file_fops = {
	.read		= bsfs_fop_read,
	.write		= bsfs_fop_write,
	.llseek		= generic_file_llseek,
	.mmap		= bsfs_fop_mmap,
	.open		= bsfs_fop_open,
	.release	= bsfs_fop_release,
	.poll		= bsfs_fop_poll,
	.fsync		= noop_fsync,
};
