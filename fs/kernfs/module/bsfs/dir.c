/*
 * bsfs dir
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/namei.h>

#include "linux/bsfs.h"

static DEFINE_SPINLOCK(bsfs_idr_lock);
DEFINE_MUTEX(bsfs_mutex);

#define rb_to_kn(X)	rb_entry((X), struct bsfs_node, rb)

static bool bsfs_active(struct bsfs_node *kn)
{
	lockdep_assert_held(&bsfs_mutex);
	return atomic_read(&kn->active) >= 0;
}

static inline unsigned char dt_type(struct bsfs_node *kn)
{
	return (kn->mode >> 12) & 15;
}

static bool bsfs_lockdep(struct bsfs_node *kn)
{
	return false;
}

static struct bsfs_node *__bsfs_new_node(struct bsfs_root *root,
				const char *name, umode_t mode,
				kuid_t uid, kgid_t gid,
				unsigned flags)
{
	struct bsfs_node *kn;
	u32 gen;
	int cursor;
	int ret;

	name = kstrdup_const(name, GFP_KERNEL);
	if (!name)
		return NULL;

	kn = kmem_cache_zalloc(bsfs_node_cache, GFP_KERNEL);
	if (!kn)
		goto err_out1;

	idr_preload(GFP_KERNEL);
	spin_lock(&bsfs_idr_lock);
	cursor = idr_get_cursor(&root->ino_idr);
	ret = idr_alloc_cyclic(&root->ino_idr, kn, 1, 0, GFP_ATOMIC);
	if (ret >= 0 && ret < cursor)
		root->next_generation++;
	gen = root->next_generation;
	spin_unlock(&bsfs_idr_lock);
	idr_preload_end();
	if (ret < 0)
		goto err_out2;
	kn->id.ino = ret;
	kn->id.generation = gen;

	smp_mb__before_atomic();
	atomic_set(&kn->count, 1);
	atomic_set(&kn->active, BS_DEACTIVATED_BIAS);
	RB_CLEAR_NODE(&kn->rb);

	kn->name = name;
	kn->mode = mode;
	kn->flags = flags;

	if (!uid_eq(uid, GLOBAL_ROOT_UID) || !gid_eq(gid, GLOBAL_ROOT_GID)) {
		struct iattr iattr = {
			.ia_valid = ATTR_UID | ATTR_GID,
			.ia_uid = uid,
			.ia_gid = gid,
		};
		ret = __bsfs_setattr(kn, &iattr);
		if (ret < 0)
			goto err_out3;
	}

	return kn;

err_out3:
	idr_remove(&root->ino_idr, kn->id.ino);
err_out2:
	kmem_cache_free(bsfs_node_cache, kn);
err_out1:
	kfree_const(name);
	return NULL;
}

static struct bsfs_node *bsfs_leftmost_descendant(struct bsfs_node *pos)
{
	struct bsfs_node *last;

	while (true) {
		struct rb_node *rbn;

		last = pos;

		if (bsfs_type(pos) != BSFS_DIR)
			break;

		rbn = rb_first(&pos->dir.children);
		if (!rbn)
			break;

		pos = rb_to_kn(rbn);
	}
	return last;
}

static struct bsfs_node *bsfs_next_descendant_post(struct bsfs_node *pos,
				struct bsfs_node *root)
{
	struct rb_node *rbn;

	lockdep_assert_held(&bsfs_mutex);

	if (!pos)
		return bsfs_leftmost_descendant(root);

	if (pos == root)
		return NULL;

	rbn = rb_next(&pos->rb);
	if (rbn)
		return bsfs_leftmost_descendant(rb_to_kn(rbn));

	return pos->parent;
}

void bsfs_activate(struct bsfs_node *kn)
{
	struct bsfs_node *pos;

	mutex_lock(&bsfs_mutex);

	pos = NULL;
	while ((pos = bsfs_next_descendant_post(pos, kn))) {
		if (!pos || (pos->flags & BSFS_ACTIVATED))
			continue;

		WARN_ON_ONCE(pos->parent && RB_EMPTY_NODE(&pos->rb));
		WARN_ON_ONCE(atomic_read(&pos->active) != BS_DEACTIVATED_BIAS);

		atomic_sub(BS_DEACTIVATED_BIAS, &pos->active);
		pos->flags |= BSFS_ACTIVATED;
	}
	mutex_unlock(&bsfs_mutex);
}

struct bsfs_root *bsfs_create_root(struct bsfs_syscall_ops *scops,
			unsigned int flags, void *priv)
{
	struct bsfs_root *root;
	struct bsfs_node *kn;

	root = kzalloc(sizeof(*root), GFP_KERNEL);
	if (!root)
		return ERR_PTR(-ENOMEM);

	idr_init(&root->ino_idr);
	INIT_LIST_HEAD(&root->supers);
	root->next_generation = 1;

	kn = __bsfs_new_node(root, "", S_IFDIR | S_IRUGO | S_IXUGO,
			GLOBAL_ROOT_UID, GLOBAL_ROOT_GID,
			BSFS_DIR);
	if (!kn) {
		idr_destroy(&root->ino_idr);
		kfree(root);
	}

	kn->priv = priv;
	kn->dir.root = root;

	root->syscall_ops = scops;
	root->flags = flags;
	root->kn = kn;
	init_waitqueue_head(&root->deactivate_waitq);

	if (!(root->flags & BSFS_ROOT_CREATE_DEACTIVATED))
		bsfs_activate(kn);

	return root;
}

void bsfs_put(struct bsfs_node *kn)
{
	struct bsfs_node *parent;
	struct bsfs_root *root;

	if (!kn || !atomic_dec_and_test(&kn->count))
		return;
	root = bsfs_root(kn);
repeat:
	parent = kn->parent;

	WARN_ONCE(atomic_read(&kn->active) != BS_DEACTIVATED_BIAS,
		"bsfs_put: %s/%s: released with incorrect active_ref %d\n",
		parent ? parent->name : "", 
			kn->name, atomic_read(&kn->active));
	if (bsfs_type(kn) == BSFS_LINK)
		bsfs_put(kn->symlink.target_kn);

	kfree_const(kn->name);

	if (kn->iattr) {
		if (kn->iattr->ia_secdata)
			security_release_secctx(kn->iattr->ia_secdata,
					kn->iattr->ia_secdata_len);
		simple_xattrs_free(&kn->iattr->xattrs);
	}
	kfree(kn->iattr);
	spin_lock(&bsfs_idr_lock);
	idr_remove(&root->ino_idr, kn->id.ino);
	spin_unlock(&bsfs_idr_lock);
	kmem_cache_free(bsfs_node_cache, kn);

	kn = parent;
	if (kn) {
		if (atomic_dec_and_test(&kn->count))
			goto repeat;
	} else {
		idr_destroy(&root->ino_idr);
		kfree(root);
	}
}

static bool bsfs_unlink_sibling(struct bsfs_node *kn)
{
	if (RB_EMPTY_NODE(&kn->rb))
		return false;

	if (bsfs_type(kn) == BSFS_DIR)
		kn->parent->dir.subdirs--;

	rb_erase(&kn->rb, &kn->parent->dir.children);
	RB_CLEAR_NODE(&kn->rb);
	return true;
}

static void bsfs_drain(struct bsfs_node *kn)
	__releases(&bsfs_mutex) __acquires(&bsfs_mutex)
{
	struct bsfs_root *root = bsfs_root(kn);

	lockdep_assert_held(&bsfs_mutex);
	WARN_ON_ONCE(bsfs_active(kn));

	mutex_unlock(&bsfs_mutex);

	if (bsfs_lockdep(kn)) {
		rwsem_acquire(&kn->dep_map, 0, 0, _RET_IP_);
		if (atomic_read(&kn->active) != BS_DEACTIVATED_BIAS)
			lock_contended(&kn->dep_map, _RET_IP_);
	}

	wait_event(root->deactivate_waitq,
			atomic_read(&kn->active) == BS_DEACTIVATED_BIAS);

	if (bsfs_lockdep(kn)) {
		lock_acquired(&kn->dep_map, _RET_IP_);
		rwsem_release(&kn->dep_map, 1, _RET_IP_);
	}

	bsfs_drain_open_files(kn);
	mutex_lock(&bsfs_mutex);
}

void bsfs_get(struct bsfs_node *kn)
{
	if (kn) {
		WARN_ON(!atomic_read(&kn->count));
		atomic_inc(&kn->count);
	}
}

static void __bsfs_remove(struct bsfs_node *kn)
{
	struct bsfs_node *pos;

	lockdep_assert_held(&bsfs_mutex);

	if (!kn || (kn->parent && RB_EMPTY_NODE(&kn->rb)))
		return;

	printk("bsfs %s: removing\n", kn->name);

	pos = NULL;
	while ((pos = bsfs_next_descendant_post(pos, kn)))
		if (bsfs_active(pos))
			atomic_add(BS_DEACTIVATED_BIAS, &pos->active);

	do {
		pos = bsfs_leftmost_descendant(kn);
		bsfs_get(pos);

		if (kn->flags & BSFS_ACTIVATED)
			bsfs_drain(pos);
		else
			WARN_ON_ONCE(atomic_read(&kn->active) != 
						BS_DEACTIVATED_BIAS);

		if (!pos->parent || bsfs_unlink_sibling(pos)) {
			struct bsfs_iattrs *ps_iattr = 
				pos->parent ? pos->parent->iattr : NULL;

			if (ps_iattr) {
				ktime_get_real_ts64(&pos->iattr->ia_iattr.ia_ctime);
				ps_iattr->ia_iattr.ia_mtime = 
					ps_iattr->ia_iattr.ia_ctime;
			}

			bsfs_put(pos);
		}
		bsfs_put(pos);
	} while (pos != kn);
}

void bsfs_remove(struct bsfs_node *kn)
{
	mutex_lock(&bsfs_mutex);
	__bsfs_remove(kn);
	mutex_unlock(&bsfs_mutex);
}

void bsfs_destroy_root(struct bsfs_root *root)
{
	bsfs_remove(root->kn);
}

struct bsfs_node *bsfs_find_and_get_node_by_ino(struct bsfs_root *root,
					unsigned int ino)
{
	struct bsfs_node *kn;

	rcu_read_lock();
	kn = idr_find(&root->ino_idr, ino);
	if (!kn)
		goto out;

	if (!atomic_inc_not_zero(&kn->count)) {
		kn = NULL;
		goto out;
	}

	if (kn->id.ino != ino)
		goto out;
	rcu_read_unlock();

	return kn;
out:
	rcu_read_unlock();
	bsfs_put(kn);
	return NULL;
}

static unsigned int bsfs_name_hash(const char *name, const void *ns)
{
	unsigned long hash = init_name_hash(ns);
	unsigned long len = strlen(name);

	while (len--)
		hash = partial_name_hash(*name++, hash);
	hash = end_name_hash(hash);
	hash &= 0x7FFFFFFFU;
	if (hash < 2)
		hash += 2;
	if (hash >= INT_MAX)
		hash = INT_MAX - 1;
	return hash;
}

static int bsfs_name_compare(unsigned int hash, const char *name,
			const void *ns, const struct bsfs_node *kn)
{
	if (hash < kn->hash)
		return -1;
	if (hash > kn->hash)
		return 1;
	if (ns < kn->ns)
		return -1;
	if (ns > kn->ns)
		return 1;
	return strcmp(name, kn->name);
}

static struct bsfs_node *bsfs_find_ns(struct bsfs_node *parent,
				const unsigned char *name,
				const void *ns)
{
	struct rb_node *node = parent->dir.children.rb_node;
	bool has_ns = bsfs_ns_enabled(parent);
	unsigned int hash;

	lockdep_assert_held(&bsfs_mutex);

	if (has_ns != (bool)ns) {
		WARN(1, KERN_WARNING "bsfs: ns %s in '%s' for '%s'\n",
			has_ns ? "required" : "invalid", parent->name, name);
	}

	hash = bsfs_name_hash(name, ns);
	while (node) {
		struct bsfs_node *kn;
		int result;

		kn = rb_to_kn(node);
		result = bsfs_name_compare(hash, name, ns, kn);
		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return kn;
	}
	return NULL;
}

static struct dentry *bsfs_iop_lookup(struct inode *dir,
			struct dentry *dentry, unsigned int flags)
{
	struct dentry *ret;
	struct bsfs_node *parent = dir->i_private;
	struct bsfs_node *kn;
	struct inode *inode;
	const void *ns = NULL;

	mutex_lock(&bsfs_mutex);

	if (bsfs_ns_enabled(parent))
		ns = bsfs_info(dir->i_sb)->ns;

	kn = bsfs_find_ns(parent, dentry->d_name.name, ns);

	if (!kn || !bsfs_active(kn)) {
		ret = NULL;
		goto out_unlock;
	}

	inode = bsfs_get_inode(dir->i_sb, kn);
	if (!inode) {
		ret = ERR_PTR(-ENOMEM);
		goto out_unlock;
	}

	ret = d_splice_alias(inode, dentry);
out_unlock:
	mutex_unlock(&bsfs_mutex);
	return ret;
}

void bsfs_put_active(struct bsfs_node *kn)
{
	struct bsfs_root *root = bsfs_root(kn);
	int v;

	if (unlikely(!kn))
		return;

	if (bsfs_lockdep(kn))
		rwsem_release(&kn->dep_map, 1, _RET_IP_);
	v = atomic_dec_return(&kn->active);
	if (likely(v != BS_DEACTIVATED_BIAS))
		return;

	wake_up_all(&root->deactivate_waitq);
}

struct bsfs_node *bsfs_get_active(struct bsfs_node *kn)
{
	if (unlikely(!kn))
		return NULL;

	if (!atomic_inc_unless_negative(&kn->active))
		return NULL;

	if (bsfs_lockdep(kn))
		rwsem_acquire_read(&kn->dep_map, 0, 1, _RET_IP_);
	return kn;
}

static int bsfs_iop_mkdir(struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	struct bsfs_node *parent = dir->i_private;
	struct bsfs_syscall_ops *scops = bsfs_root(parent)->syscall_ops;
	int ret;

	if (!scops || !scops->mkdir)
		return -EPERM;

	if (!bsfs_get_active(parent))
		return -ENODEV;

	ret = scops->mkdir(parent, dentry->d_name.name, mode);

	bsfs_put_active(parent);
	return ret;
} 

static int bsfs_iop_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct bsfs_node *kn = bsfs_dentry_node(dentry);
	struct bsfs_syscall_ops *scops = bsfs_root(kn)->syscall_ops;
	int ret;

	if (!scops || !scops->rmdir)
		return -EPERM;

	if (!bsfs_get_active(kn))
		return -ENODEV;

	ret = scops->rmdir(kn);

	bsfs_put_active(kn);
	return ret;
}

static int bsfs_iop_rename(struct inode *old_dir, struct dentry *old_dentry,
			struct inode *new_dir, struct dentry *new_dentry,
			unsigned int flags)
{
	struct bsfs_node *kn = bsfs_dentry_node(old_dentry);
	struct bsfs_node *new_parent = new_dir->i_private;
	struct bsfs_syscall_ops *scops = bsfs_root(kn)->syscall_ops;
	int ret;

	if (flags)
		return -EINVAL;

	if (!scops || !scops->rename)
		return -EPERM;

	if (!bsfs_get_active(kn))
		return -ENODEV;

	if (!bsfs_get_active(new_parent)) {
		bsfs_put_active(kn);
		return -ENODEV;
	}

	ret = scops->rename(kn, new_parent, new_dentry->d_name.name);

	bsfs_put_active(new_parent);
	bsfs_put_active(kn);
	return ret;
}

static struct bsfs_node *bsfs_dir_pos(const void *ns,
	struct bsfs_node *parent, loff_t hash, struct bsfs_node *pos)
{
	if (pos) {
		int valid = bsfs_active(pos) &&
			pos->parent == parent && hash == pos->hash;
		bsfs_put(pos);
		if (!valid)
			pos = NULL;
	}
	if (!pos && (hash > 1) && (hash < INT_MAX)) {
		struct rb_node *node = parent->dir.children.rb_node;
		while (node) {
			pos = rb_to_kn(node);

			if (hash < pos->hash)
				node = node->rb_left;
			else if (hash > pos->hash)
				node = node->rb_right;
			else
				break;
		}
	}

	while (pos && (!bsfs_active(pos) || pos->ns != ns)) {
		struct rb_node *node = rb_next(&pos->rb);
		if (!node)
			pos = NULL;
		else
			pos = rb_to_kn(node);
	}
	return pos;
}

static struct bsfs_node *bsfs_dir_next_pos(const void *ns,
	struct bsfs_node *parent, ino_t ino, struct bsfs_node *pos)
{
	pos = bsfs_dir_pos(ns, parent, ino, pos);
	if (pos) {
		do {
			struct rb_node *node = rb_next(&pos->rb);
			if (!node)
				pos = NULL;
			else
				pos = rb_to_kn(node);
		} while (pos && (!bsfs_active(pos) || pos->ns != ns));
	}
	return pos;
}

static int bsfs_fop_readdir(struct file *file, struct dir_context *ctx)
{
	struct dentry *dentry = file->f_path.dentry;
	struct bsfs_node *parent = bsfs_dentry_node(dentry);
	struct bsfs_node *pos = file->private_data;
	const void *ns = NULL;

	if (!dir_emit_dots(file, ctx))
		return 0;
	mutex_lock(&bsfs_mutex);

	if (bsfs_ns_enabled(parent))
		ns = bsfs_info(dentry->d_sb)->ns;

	for (pos = bsfs_dir_pos(ns, parent, ctx->pos, pos);
	     pos;
	     pos = bsfs_dir_next_pos(ns, parent, ctx->pos, pos)) {
		const char *name = pos->name;
		unsigned int type = dt_type(pos);
		int len = strlen(name);
		ino_t ino = pos->id.ino;

		ctx->pos = pos->hash;
		file->private_data = pos;
		bsfs_get(pos);

		mutex_unlock(&bsfs_mutex);
		if (!dir_emit(ctx, name, len, ino, type))
			return 0;
		mutex_lock(&bsfs_mutex);
	}
	mutex_unlock(&bsfs_mutex);
	file->private_data = NULL;
	ctx->pos = INT_MAX;
	return 0;
}

static int bsfs_dir_fop_release(struct inode *inode, struct file *filp)
{
	bsfs_put(filp->private_data);
	return 0;
}

static int bsfs_dop_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct bsfs_node *kn;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	if (d_really_is_negative(dentry))
		goto out_bad_unlocked;

	kn = bsfs_dentry_node(dentry);
	mutex_lock(&bsfs_mutex);

	if (!bsfs_active(kn))
		goto out_bad;

	if (bsfs_dentry_node(dentry->d_parent) != kn->parent)
		goto out_bad;

	if (strcmp(dentry->d_name.name, kn->name) != 0)
		goto out_bad;

	if (kn->parent && bsfs_ns_enabled(kn->parent) &&
		bsfs_info(dentry->d_sb)->ns != kn->ns)
		goto out_bad;

	mutex_unlock(&bsfs_mutex);
	return 1;
out_bad:
	mutex_unlock(&bsfs_mutex);
out_bad_unlocked:
	return 0;
}

const struct inode_operations bsfs_dir_iops = {
	.lookup		= bsfs_iop_lookup,
	.permission	= bsfs_iop_permission,
	.setattr	= bsfs_iop_setattr,
	.getattr	= bsfs_iop_getattr,
	.listxattr	= bsfs_iop_listxattr,

	.mkdir		= bsfs_iop_mkdir,
	.rmdir		= bsfs_iop_rmdir,
	.rename		= bsfs_iop_rename,
};

const struct file_operations bsfs_dir_fops = {
	.read		= generic_read_dir,
	.iterate_shared	= bsfs_fop_readdir,
	.release	= bsfs_dir_fop_release,
	.llseek		= generic_file_llseek,
};

const struct dentry_operations bsfs_dops = {
	.d_revalidate	= bsfs_dop_revalidate,
};
