/*
 * inode
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/security.h>
#include <linux/mm.h>

#include "linux/bsfs.h"

static const struct address_space_operations bsfs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
};

static struct bsfs_iattrs *bsfs_iattrs(struct bsfs_node *kn)
{
	static DEFINE_MUTEX(iattr_mutex);
	struct bsfs_iattrs *ret;
	struct iattr *iattrs;

	mutex_lock(&iattr_mutex);

	if (kn->iattr)
		goto out_unlock;

	kn->iattr = kzalloc(sizeof(struct bsfs_iattrs), GFP_KERNEL);
	if (!kn->iattr)
		goto out_unlock;
	iattrs = &kn->iattr->ia_iattr;

	/* assign default attributes */
	iattrs->ia_mode = kn->mode;
	iattrs->ia_uid = GLOBAL_ROOT_UID;
	iattrs->ia_gid = GLOBAL_ROOT_GID;

	ktime_get_real_ts64(&iattrs->ia_atime);
	iattrs->ia_mtime = iattrs->ia_atime;
	iattrs->ia_ctime = iattrs->ia_atime;

	simple_xattrs_init(&kn->iattr->xattrs);
out_unlock:
	ret = kn->iattr;
	mutex_unlock(&iattr_mutex);
	return ret;
}

int __bsfs_setattr(struct bsfs_node *kn, const struct iattr *iattr)
{
	struct bsfs_iattrs *attrs;
	struct iattr *iattrs;
	unsigned int ia_valid = iattr->ia_valid;

	attrs = bsfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	iattrs = &attrs->ia_iattr;

	if (ia_valid & ATTR_UID)
		iattrs->ia_uid = iattr->ia_uid;
	if (ia_valid & ATTR_GID)
		iattrs->ia_gid = iattr->ia_gid;
	if (ia_valid & ATTR_ATIME)
		iattrs->ia_atime = iattr->ia_atime;
	if (ia_valid & ATTR_MTIME)
		iattrs->ia_mtime = iattr->ia_mtime;
	if (ia_valid & ATTR_CTIME)
		iattrs->ia_ctime = iattr->ia_ctime;
	if (ia_valid & ATTR_MODE) {
		umode_t mode = iattr->ia_mode;
		iattrs->ia_mode = kn->mode = mode;
	}
	return 0;
}

void bsfs_evict_inode(struct inode *inode)
{
	struct bsfs_node *kn = inode->i_private;

	truncate_inode_pages_final(&inode->i_data);
	clear_inode(inode);
	bsfs_put(kn);
}

static int bsfs_xattr_set(const struct xattr_handler *handler,
			struct dentry *unused, struct inode *inode,
			const char *suffix, const void *value,
			size_t size, int flags)
{
	const char *name = xattr_full_name(handler, suffix);
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_iattrs *attrs;

	attrs = bsfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_set(&attrs->xattrs, name, value, size, flags);
}

static int bsfs_xattr_get(const struct xattr_handler *handler,
			struct dentry *unused, struct inode *inode,
			const char *suffix, void *value, size_t size)
{
	const char *name = xattr_full_name(handler, suffix);
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_iattrs *attrs;

	attrs = bsfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_get(&attrs->xattrs, name, value, size);
}

static int bsfs_node_setsecdata(struct bsfs_iattrs *attrs, void **secdata,
				u32 *secdata_len)
{
	void *old_secdata;
	size_t old_secdata_len;

	old_secdata = attrs->ia_secdata;
	old_secdata_len = attrs->ia_secdata_len;

	attrs->ia_secdata = *secdata;
	attrs->ia_secdata_len = *secdata_len;

	*secdata = old_secdata;
	*secdata_len = old_secdata_len;
	return 0;
}

static int bsfs_security_xattr_set(const struct xattr_handler *handler,
			struct dentry *unused, struct inode *inode,
			const char *suffix, const void *value,
			size_t size, int flags)
{
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_iattrs *attrs;
	void *secdata;
	u32 secdata_len = 0;
	int error;

	attrs = bsfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;
	error = security_inode_setsecurity(inode, suffix, value, size, flags);
	if (error)
		return error;
	error = security_inode_getsecctx(inode, &secdata, &secdata_len);
	if (error)
		return error;

	mutex_lock(&bsfs_mutex);
	error = bsfs_node_setsecdata(attrs, &secdata, &secdata_len);
	mutex_unlock(&bsfs_mutex);

	if (secdata)
		security_release_secctx(secdata, secdata_len);
	return error;
}

static inline void set_inode_attr(struct inode *inode, struct iattr *iattr)
{
	struct super_block *sb = inode->i_sb;

	inode->i_uid = iattr->ia_uid;
	inode->i_gid = iattr->ia_gid;
	inode->i_atime = timespec64_trunc(iattr->ia_atime, sb->s_time_gran);
	inode->i_mtime = timespec64_trunc(iattr->ia_mtime, sb->s_time_gran);
	inode->i_ctime = timespec64_trunc(iattr->ia_ctime, sb->s_time_gran);
}

static void bsfs_refresh_inode(struct bsfs_node *kn, struct inode *inode)
{
	struct bsfs_iattrs *attrs = kn->iattr;

	inode->i_mode = kn->mode;
	if (attrs) {
		set_inode_attr(inode, &attrs->ia_iattr);
		security_inode_notifysecctx(inode, attrs->ia_secdata,
					attrs->ia_secdata_len);
	}

	if (bsfs_type(kn) == BSFS_DIR)
		set_nlink(inode, kn->dir.subdirs + 2);
}

int bsfs_iop_permission(struct inode *inode, int mask)
{
	struct bsfs_node *kn;

	if (mask & MAY_NOT_BLOCK)
		return -ECHILD;

	kn = inode->i_private;

	mutex_lock(&bsfs_mutex);
	bsfs_refresh_inode(kn, inode);
	mutex_unlock(&bsfs_mutex);

	return generic_permission(inode, mask);
}

int bsfs_iop_setattr(struct dentry *dentry, struct iattr *iattr)
{
	struct inode *inode = d_inode(dentry);
	struct bsfs_node *kn = inode->i_private;
	int error;

	if (!kn)
		return -EINVAL;

	mutex_lock(&bsfs_mutex);
	error = setattr_prepare(dentry, iattr);
	if (error)
		goto out;

	error = __bsfs_setattr(kn, iattr);
	if (error)
		goto out;

	setattr_copy(inode, iattr);

out:
	mutex_unlock(&bsfs_mutex);
	return error;
}

int bsfs_iop_getattr(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct bsfs_node *kn = inode->i_private;

	mutex_lock(&bsfs_mutex);
	bsfs_refresh_inode(kn, inode);
	mutex_unlock(&bsfs_mutex);

	generic_fillattr(inode, stat);
	return 0;
}

ssize_t bsfs_iop_listxattr(struct dentry *dentry, char *buf, size_t size)
{
	struct bsfs_node *kn = bsfs_dentry_node(dentry);
	struct bsfs_iattrs *attrs;

	attrs = bsfs_iattrs(kn);
	if (!attrs)
		return -ENOMEM;

	return simple_xattr_list(d_inode(dentry), &attrs->xattrs, buf, size);
}

static inline void set_default_inode_attr(struct inode *inode, umode_t mode)
{
	inode->i_mode = mode;
	inode->i_atime = inode->i_mtime = 
		inode->i_ctime = current_time(inode);
}

static const struct inode_operations bsfs_iops = {
	.permission	= bsfs_iop_permission,
	.setattr	= bsfs_iop_setattr,
	.getattr	= bsfs_iop_getattr,
	.listxattr	= bsfs_iop_listxattr,
};

static void bsfs_init_inode(struct bsfs_node *kn, struct inode *inode)
{
	bsfs_get(kn);
	inode->i_private = kn;
	inode->i_mapping->a_ops = &bsfs_aops;
	inode->i_op = &bsfs_iops;
	inode->i_generation = kn->id.generation;

	set_default_inode_attr(inode, kn->mode);
	bsfs_refresh_inode(kn, inode);

	/* initialize inode according to type */
	switch (bsfs_type(kn)) {
	case BSFS_DIR:
		inode->i_op = &bsfs_dir_iops;
		inode->i_fop = &bsfs_dir_fops;
		if (kn->flags & BSFS_EMPTY_DIR)
			make_empty_dir_inode(inode);
		break;
	case BSFS_FILE:
		inode->i_size = kn->attr.size;
		inode->i_fop = &bsfs_file_fops;
		break;
	case BSFS_LINK:
		inode->i_op = &bsfs_symlink_iops;
		break;
	default:
		BUG();
	}

	unlock_new_inode(inode);
}

struct inode *bsfs_get_inode(struct super_block *sb, struct bsfs_node *kn)
{
	struct inode *inode;

	inode = iget_locked(sb, kn->id.ino);
	if (inode && (inode->i_state & I_NEW))
		bsfs_init_inode(kn, inode);

	return inode;
}

static const struct xattr_handler bsfs_trusted_xattr_handler = {
	.prefix = XATTR_TRUSTED_PREFIX,
	.get = bsfs_xattr_get,
	.set = bsfs_xattr_set,
};

static const struct xattr_handler bsfs_security_xattr_handler = {
	.prefix = XATTR_SECURITY_PREFIX,
	.get = bsfs_xattr_get,
	.set = bsfs_security_xattr_set,
};

const struct xattr_handler *bsfs_xattr_handlers[] = {
	&bsfs_trusted_xattr_handler,
	&bsfs_security_xattr_handler,
	NULL,
};
