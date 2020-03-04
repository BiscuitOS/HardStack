/*
 * Proc filesytem
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/pid_namespace.h>
#include <uapi/linux/magic.h>

#include "internal.h"

static struct kmem_cache *proc_inode_cachep_bs __ro_after_init;
static struct kmem_cache *pde_opener_cache_bs __ro_after_init;

static const char *proc_get_link_bs(struct dentry *dentry,
				    struct inode *inode,
				    struct delayed_call *done)
{
	BS_DUP();
	return NULL;
}

static struct inode *proc_alloc_inode_bs(struct super_block *sb)
{
	struct proc_inode *ei;

	ei = kmem_cache_alloc(proc_inode_cachep_bs, GFP_KERNEL);
	if (!ei)
		return NULL;

	ei->pid = NULL;
	ei->fd = 0;
	ei->op.proc_get_link = NULL;
	ei->pde = NULL;
	ei->sysctl = NULL;
	ei->sysctl_entry = NULL;
	ei->ns_ops = NULL;
	return &ei->vfs_inode; 
}

static void proc_destroy_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static void proc_evict_inode_bs(struct inode *inode)
{
	struct proc_dir_entry *de;
	struct ctl_table_header *head;

	truncate_inode_pages_final(&inode->i_data);
	clear_inode(inode);

	/* Stop tracking associated processes */
	put_pid(PROC_I_BS(inode)->pid);

	/* Let go of any associated proc directory entry */
	de = PDE_BS(inode);
	if (de)
		pde_put_bs(de);

	head = PROC_I_BS(inode)->sysctl;
	if (head) {
		RCU_INIT_POINTER(PROC_I_BS(inode)->sysctl, NULL);
		proc_sys_evict_inode_bs(inode, head);
	}
}

static int proc_show_options_bs(struct seq_file *seq, struct dentry *root)
{
	BS_DUP();
	return 0;
}

static loff_t proc_reg_llseek_bs(struct file *file, loff_t offset, int whence)
{
	BS_DUP();
	return 0;
}

static ssize_t proc_reg_read_bs(struct file *file, char __user *buf,
						size_t count, loff_t *ppos)
{
	BS_DUP();
	return 0;
}

static ssize_t proc_reg_write_bs(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	BS_DUP();
	return 0;
}

static __poll_t proc_reg_poll_bs(struct file *file, 
					struct poll_table_struct *pts)
{
	BS_DUP();
	return 0;
}

static long proc_reg_unlocked_ioctl_bs(struct file *file,
				unsigned int cmd, unsigned long arg)
{
	BS_DUP();
	return 0;
}

static int proc_reg_mmap_bs(struct file *file, struct vm_area_struct *vma)
{
	BS_DUP();
	return 0;
}

static unsigned long
proc_reg_get_unmapped_area_bs(struct file *file, unsigned long orig_addr,
			unsigned long len, unsigned long pgoff,
			unsigned long flags)
{
	BS_DUP();
	return 0;
}

static int proc_reg_open_bs(struct inode *inode, struct file *file)
{
	BS_DUP();
	return 0;
}

static int proc_reg_release_bs(struct inode *inode, struct file *file)
{
	BS_DUP();
	return 0;
}

static const struct file_operations proc_reg_file_ops_bs = {
	.llseek		= proc_reg_llseek_bs,
	.read		= proc_reg_read_bs,
	.write		= proc_reg_write_bs,
	.poll		= proc_reg_poll_bs,
	.unlocked_ioctl	= proc_reg_unlocked_ioctl_bs,
	.mmap		= proc_reg_mmap_bs,
	.get_unmapped_area = proc_reg_get_unmapped_area_bs,
	.open		= proc_reg_open_bs,
	.release	= proc_reg_release_bs,
};

struct inode *proc_get_inode_bs(struct super_block *sb, 
						struct proc_dir_entry *de)
{
	struct inode *inode = new_inode_pseudo(sb);

	if (inode) {
		inode->i_ino = de->low_ino;
		inode->i_mtime = inode->i_atime = 
					inode->i_ctime = current_time(inode);
		PROC_I_BS(inode)->pde = de;

		if (is_empty_pde_bs(de)) {
			make_empty_dir_inode(inode);
			return inode;
		}
		if (de->mode) {
			inode->i_mode = de->mode;
			inode->i_uid  = de->uid;
			inode->i_gid  = de->gid;
		}
		if (de->size)
			inode->i_size = de->size;
		if (de->nlink)
			set_nlink(inode, de->nlink);
		WARN_ON(!de->proc_iops);
		inode->i_op = de->proc_iops;
		if (de->proc_fops) {
			if (S_ISREG(inode->i_mode)) {
				inode->i_fop = &proc_reg_file_ops_bs;
			} else {
				inode->i_fop = de->proc_fops;
			}
		}
	} else
		pde_put_bs(de);
	return inode;
}

const struct inode_operations proc_link_inode_operations_bs = {
	.get_link	= proc_get_link_bs,
};

static const struct super_operations proc_sops_bs = {
	.alloc_inode	= proc_alloc_inode_bs,
	.destroy_inode	= proc_destroy_inode_bs,
	.drop_inode	= generic_delete_inode,
	.evict_inode	= proc_evict_inode_bs,
	.statfs		= simple_statfs,
	.remount_fs	= proc_remount_bs,
	.show_options	= proc_show_options_bs,
};

static void init_once(void *foo)
{
	struct proc_inode *ei = (struct proc_inode *) foo;

	inode_init_once(&ei->vfs_inode);
}

void __init proc_init_kmemcache_bs(void)
{
	proc_inode_cachep_bs = kmem_cache_create("proc_inode_cache_bs",
						sizeof(struct proc_inode),
						0,
						(SLAB_RECLAIM_ACCOUNT|
						SLAB_MEM_SPREAD|SLAB_ACCOUNT|
						SLAB_PANIC),
						init_once);
	pde_opener_cache_bs = kmem_cache_create("pde_opener_bs", 
						sizeof(struct pde_opener), 
						0,
				  		SLAB_ACCOUNT|SLAB_PANIC, 
						NULL);
	proc_dir_entry_cache_bs = kmem_cache_create_usercopy(
						"proc_dir_entry_bs",
						SIZEOF_PDE,
						0,
						SLAB_PANIC,
						offsetof(struct proc_dir_entry,
							inline_name),
						SIZEOF_PDE_INLINE_NAME,
						NULL);
	BUILD_BUG_ON(sizeof(struct proc_dir_entry) >= SIZEOF_PDE);
}

int proc_fill_super_bs(struct super_block *s, void *data, int silent)
{
	struct pid_namespace *ns = get_pid_ns(s->s_fs_info);
	struct inode *root_inode;
	int ret;

	if (!proc_parse_options_bs(data, ns))
		return -EINVAL;

	/* User space would break if executable or devices appear on proc */
	s->s_iflags |= SB_I_USERNS_VISIBLE | SB_I_NOEXEC | SB_I_NODEV;
	s->s_flags  |= SB_NODIRATIME | SB_NOSUID | SB_NOEXEC;
	s->s_blocksize = 1024;
	s->s_blocksize_bits = 10;
	s->s_magic = PROC_SUPER_MAGIC | BISCUITOS_PROC_MAGIC;
	s->s_op = &proc_sops_bs;
	s->s_time_gran = 1;

	/*
	 * procfs isn't actually a stacking filesystem; however, there is
	 * too much magic going on inside it to permit stacking thing on
	 * top of it.
	 */
	s->s_stack_depth = FILESYSTEM_MAX_STACK_DEPTH;

	/* procfs dentries and inodes don't require ID to create. */
	s->s_shrink.seeks = 0;

	pde_get_bs(&proc_root_bs);
	root_inode = proc_get_inode_bs(s, &proc_root_bs);
	if (!root_inode) {
		pr_err("proc_fill_super: get root inode failed\n");
		return -ENOMEM;
	}

	s->s_root = d_make_root(root_inode);
	if (!s->s_root) {
		pr_err("proc_fill_super: allocate dentry failed\n");
		return -ENOMEM;
	}

	ret = proc_setup_self_bs(s);
	if (ret) {
		return ret;
	}

	return proc_setup_thread_self_bs(s);
}
