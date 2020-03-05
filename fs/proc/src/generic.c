/*
 * Proc filesytem --- generic routines for the proc-fs
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/string.h>
#include <linux/namei.h>

#include "internal.h"

struct kmem_cache *proc_dir_entry_cache_bs __ro_after_init;
static DEFINE_IDA(proc_inum_ida_bs);
static DEFINE_RWLOCK(proc_subdir_lock_bs);

#define PROC_DYNAMIC_FIRST	0xF0000000U

void pde_free_bs(struct proc_dir_entry *pde)
{
	if (S_ISLNK(pde->mode))
		kfree(pde->data);
	if (pde->name != pde->inline_name)
		kfree(pde->name);
	kmem_cache_free(proc_dir_entry_cache_bs, pde);
}

/*
 * Return an inode number between PROC_DYNAMIC_FIRST and
 * 0xffffffff, or zero on failure.
 */
int proc_alloc_inum_bs(unsigned int *inum)
{
	int i;

	i = ida_simple_get(&proc_inum_ida_bs, 0, 
			UINT_MAX - PROC_DYNAMIC_FIRST + 1, GFP_KERNEL);
	if (i < 0)
		return i;

	*inum = PROC_DYNAMIC_FIRST + (unsigned int)i;
	return 0;
}

static int proc_match_bs(const char *name, struct proc_dir_entry *de,
						unsigned int len)
{
	if (len < de->namelen)
		return -1;
	if (len > de->namelen)
		return 1;

	return memcmp(name, de->name, len);
}

static struct proc_dir_entry *pde_subdir_find_bs(struct proc_dir_entry *dir,
				const char *name, unsigned int len)
{
	struct rb_node *node = dir->subdir.rb_node;

	while (node) {
		struct proc_dir_entry *de = rb_entry(node,
						struct proc_dir_entry,
						subdir_node);
		int result = proc_match_bs(name, de, len);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return de;
	}
	return NULL;
}

/*
 * This function parses a name such as "tty/driver/serial", and
 * returns the struct proc_dir_entry for "/proc/tty/driver", and
 * returns "serial" in residual.
 */
static int __xlate_proc_name_bs(const char *name, struct proc_dir_entry **ret,
				const char **residual)
{
	const char *cp = name, *next;
	struct proc_dir_entry *de;
	unsigned int len;

	de = *ret;
	if (!de)
		de = &proc_root_bs;

	while (1) {
		next = strchr(cp, '/');
		if (!next)
			break;

		len = next - cp;
		de = pde_subdir_find_bs(de, cp, len);
		if (!de) {
			WARN(1, "name '%s'\n", name);
			return -ENOENT;
		}
		cp += len + 1;
	}
	*residual = cp;
	*ret = de;
	return 0;
}

static int xlate_proc_name_bs(const char *name, struct proc_dir_entry **ret,
			const char **residual)
{
	int rv;

	read_lock(&proc_subdir_lock_bs);
	rv = __xlate_proc_name_bs(name, ret, residual);
	read_unlock(&proc_subdir_lock_bs);
	return rv;
}

void proc_set_user_bs(struct proc_dir_entry *de, kuid_t uid, kgid_t gid)
{
	de->uid = uid;
	de->gid = gid;
}

static int proc_misc_d_revalidate_bs(struct dentry *dentry, unsigned int flags)
{
	if (flags & LOOKUP_RCU)
		return -ECHILD;
	if (atomic_read(&PDE_BS(d_inode(dentry))->in_use) < 0)
		return 0; /* revalidate */
	return 1;
}

static int proc_misc_d_delete_bs(const struct dentry *dentry)
{
	return atomic_read(&PDE_BS(d_inode(dentry))->in_use) < 0;
}

static const struct dentry_operations proc_misc_dentry_ops_bs = {
	.d_revalidate	= proc_misc_d_revalidate_bs,
	.d_delete	= proc_misc_d_delete_bs,
};

static struct proc_dir_entry *__proc_create_bs(struct proc_dir_entry **parent,
			const char *name, umode_t mode, nlink_t nlink)
{
	struct proc_dir_entry *ent = NULL;
	const char *fn = NULL;
	struct qstr qstr;

	if (xlate_proc_name_bs(name, parent, &fn) != 0)
		goto out;
	qstr.name = fn;
	qstr.len = strlen(fn);
	if (qstr.len == 0 || qstr.len >= 256) {
		WARN(1, "name len %u\n", qstr.len);
		return NULL;
	}
	if (qstr.len == 1 && fn[0] == '.') {
		WARN(1, "name '.'\n");
		return NULL;
	}
	if (qstr.len == 2 && fn[0] == '.' && fn[1] == '.') {
		WARN(1, "name '..'\n");
		return NULL;
	}
	if (*parent == &proc_root_bs && name_to_int_bs(&qstr) != ~0U) {
		WARN(1, "create '/proc_bs/%s' by hand\n", qstr.name);
		return NULL;
	}
	if (is_empty_pde_bs(*parent)) {
		WARN(1, "attempt to add to permanently empty directory");
		return NULL;
	}

	ent = kmem_cache_zalloc(proc_dir_entry_cache_bs, GFP_KERNEL);
	if (!ent)
		goto out;

	if (qstr.len + 1 <= SIZEOF_PDE_INLINE_NAME) {
		ent->name = ent->inline_name;
	} else {
		ent->name = kmalloc(qstr.len + 1, GFP_KERNEL);
		if (!ent->name) {
			BS_DUP();
			return NULL;
		}
	}

	memcpy(ent->name, fn, qstr.len + 1);
	ent->namelen = qstr.len;
	ent->mode = mode;
	ent->nlink = nlink;
	ent->subdir = RB_ROOT;
	refcount_set(&ent->refcnt, 1);
	spin_lock_init(&ent->pde_unload_lock);
	INIT_LIST_HEAD(&ent->pde_openers);
	proc_set_user_bs(ent, (*parent)->uid, (*parent)->gid);

	ent->proc_dops = &proc_misc_dentry_ops_bs;

out:
	return ent;
}

void proc_free_inum_bs(unsigned int inum)
{
	ida_simple_remove(&proc_inum_ida_bs, inum - PROC_DYNAMIC_FIRST);
}

static bool pde_subdir_insert_bs(struct proc_dir_entry *dir,
				 struct proc_dir_entry *de)
{
	struct rb_root *root = &dir->subdir;
	struct rb_node **new = &root->rb_node, *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct proc_dir_entry *this = rb_entry(*new,
						       struct proc_dir_entry,
						       subdir_node);
		int result = proc_match_bs(de->name, this, de->namelen);

		parent = *new;
		if (result < 0)
			new = &(*new)->rb_left;
		else if (result > 0)
			new = &(*new)->rb_right;
		else
			return false;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&de->subdir_node, parent, new);
	rb_insert_color(&de->subdir_node, root);
	return true;
}

/* returns the registered entry, or frees dp and returns NULL on failure */
struct proc_dir_entry *proc_register_bs(struct proc_dir_entry *dir,
						struct proc_dir_entry *dp)
{
	if (proc_alloc_inum_bs(&dp->low_ino))
		goto out_free_entry;

	write_lock(&proc_subdir_lock_bs);
	dp->parent = dir;
	if (pde_subdir_insert_bs(dir, dp) == false) {
		WARN(1, "proc_dir_entru '%s/%s' already registered\n",
			dir->name, dp->name);
		write_unlock(&proc_subdir_lock_bs);
		goto out_free_inum;
	}
	write_unlock(&proc_subdir_lock_bs);

	return dp;
out_free_inum:
	proc_free_inum_bs(dp->low_ino);
out_free_entry:
	pde_free_bs(dp);
	return NULL;
}

struct proc_dir_entry *proc_symlink_bs(const char *name,
		struct proc_dir_entry *parent, const char *dest)
{
	struct proc_dir_entry *ent;

	ent = __proc_create_bs(&parent, name,
			(S_IFLNK | S_IRUGO | S_IWUGO | S_IXUGO), 1);

	if (ent) {
		ent->data = kmalloc((ent->size = strlen(dest)) + 1, GFP_KERNEL);
		if (ent->data) {
			strcpy((char *)ent->data, dest);
			ent->proc_iops = &proc_link_inode_operations_bs;
			ent = proc_register_bs(parent, ent);
		} else {
			pde_free_bs(ent);
			ent = NULL;
		}
	}
	return ent;
}

static struct proc_dir_entry *pde_subdir_first_bs(struct proc_dir_entry *dir)
{
	return rb_entry_safe(rb_first(&dir->subdir), struct proc_dir_entry,
						subdir_node);
}

static struct proc_dir_entry *pde_subdir_next_bs(struct proc_dir_entry *dir)
{
	return rb_entry_safe(rb_next(&dir->subdir_node), struct proc_dir_entry,
						subdir_node);
}

/*
 * This returns non-zero if a EOF, so that the /proc_bs
 * root directory can use this and check if it should
 * continue with the <pid> entries..
 *
 * Note that the VFS-layout doesn't care about the return
 * value of the readdir() call, as long as it's non-negative
 * for success....
 */
int proc_readdir_de_bs(struct file *file, struct dir_context *ctx,
				struct proc_dir_entry *de)
{
	int i;

	if (!dir_emit_dots(file, ctx))
		return 0;

	i = ctx->pos - 2;
	read_lock(&proc_subdir_lock_bs);
	de = pde_subdir_first_bs(de);
	for (;;) {
		if (!de) {
			read_unlock(&proc_subdir_lock_bs);
			return 0;
		}
		if (!i)
			break;
		de = pde_subdir_next_bs(de);
		i--;
	}

	do {
		struct proc_dir_entry *next;

		pde_get_bs(de);
		read_unlock(&proc_subdir_lock_bs);
		if (!dir_emit(ctx, de->name, de->namelen,
				de->low_ino, de->mode >> 12)) {
			pde_put_bs(de);
			return 0;
		}
		ctx->pos++;
		read_lock(&proc_subdir_lock_bs);
		next = pde_subdir_next_bs(de);
		pde_put_bs(de);
		de = next;
	} while (de);
	read_unlock(&proc_subdir_lock_bs);
	return 1;
}

int proc_readdir_bs(struct file *file, struct dir_context *ctx)
{
	struct inode *inode = file_inode(file);

	return proc_readdir_de_bs(file, ctx, PDE_BS(inode));
}

/*
 * Don't create negative dentries here, return -ENOENT by hand
 * instead.
 */
struct dentry *proc_lookup_de_bs(struct inode *dir, struct dentry *dentry,
					struct proc_dir_entry *de)
{
	struct inode *inode;

	read_lock(&proc_subdir_lock_bs);
	de = pde_subdir_find_bs(de, dentry->d_name.name, dentry->d_name.len);
	if (de) {
		pde_get_bs(de);
		read_unlock(&proc_subdir_lock_bs);
		inode = proc_get_inode_bs(dir->i_sb, de);
		if (!inode)
			return ERR_PTR(-ENOMEM);
		d_set_d_op(dentry, de->proc_dops);
		return d_splice_alias(inode, dentry);
	}
	read_unlock(&proc_subdir_lock_bs);
	return ERR_PTR(-ENOENT);
}

struct dentry *proc_lookup_bs(struct inode *dir, struct dentry *dentry,
			unsigned int flags)
{
	return proc_lookup_de_bs(dir, dentry, PDE_BS(dir));
}

static int proc_getattr_bs(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct proc_dir_entry *de = PDE_BS(inode);

	if (de && de->nlink)
		set_nlink(inode, de->nlink);

	generic_fillattr(inode, stat);
	return 0;
}

static int proc_notify_change_bs(struct dentry *dentry, struct iattr *iattr)
{
	BS_DUP();
	return 0;
}

/*
 * These are the generic /proc directory operations. They
 * use the in-memory "struct proc_dir_entry" tree to parse
 * the /proc directory.
 */
static const struct file_operations proc_dir_operations_bs = {
	.llseek			= generic_file_llseek,
	.read			= generic_read_dir,
	.iterate_shared		= proc_readdir_bs,
};

/*
 * proc directories can do almost nothing..
 */
static const struct inode_operations proc_dir_inode_operations_bs = {
	.lookup			= proc_lookup_bs,
	.getattr		= proc_getattr_bs,
	.setattr		= proc_notify_change_bs,
};

struct proc_dir_entry *proc_mkdir_data_bs(const char *name, umode_t mode,
			struct proc_dir_entry *parent, void *data)
{
	struct proc_dir_entry *ent;

	if (mode == 0)
		mode = S_IRUGO | S_IXUGO;

	ent = __proc_create_bs(&parent, name, S_IFDIR | mode, 2);
	if (ent) {
		ent->data = data;
		ent->proc_fops = &proc_dir_operations_bs;
		ent->proc_iops = &proc_dir_inode_operations_bs;
		parent->nlink++;
		ent = proc_register_bs(parent, ent);
		if (!ent)
			parent->nlink--;
	}
	return ent;
}

struct proc_dir_entry *proc_mkdir_bs(const char *name,
					struct proc_dir_entry *parent)
{
	return proc_mkdir_data_bs(name, 0, parent, NULL);
}

struct proc_dir_entry *proc_create_mount_point_bs(const char *name)
{
	umode_t mode = S_IFDIR | S_IRUGO | S_IXUGO;
	struct proc_dir_entry *ent, *parent = NULL;

	ent = __proc_create_bs(&parent, name, mode, 2);
	if (ent) {
		ent->data = NULL;
		ent->proc_fops = NULL;
		ent->proc_iops = NULL;
		parent->nlink++;
		ent = proc_register_bs(parent, ent);
		if (!ent)
			parent->nlink--;
	}
	return ent;
}

void pde_put_bs(struct proc_dir_entry *pde)
{
	if (refcount_dec_and_test(&pde->refcnt)) {
		proc_free_inum_bs(pde->low_ino);
		pde_free_bs(pde);
	}
}
