/*
 * symlink
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "linux/bsfs.h"

static int bsfs_get_target_path(struct bsfs_node *parent,
				struct bsfs_node *target, char *path)
{
	struct bsfs_node *base, *kn;
	char *s = path;
	int len = 1;

	base = parent;
	while (base->parent) {
		kn = target->parent;
		while (kn->parent && base !=kn)
			kn = kn->parent;

		if (base == kn)
			break;

		if ((s - path) + 3 >= PATH_MAX)
			return -ENAMETOOLONG;

		strcpy(s, "../");
		s += 3;
		base = base->parent;
	}

	kn = target;
	while (kn->parent && kn != base) {
		len += strlen(kn->name) + 1;
		kn = kn->parent;
	}

	if (len < 2)
		return -EINVAL;
	len--;
	if ((s - path) + len >= PATH_MAX)
		return -ENAMETOOLONG;

	kn = target;
	while (kn->parent && kn != base) {
		int slen = strlen(kn->name);

		len -= slen;
		memcpy(s + len, kn->name, slen);
		if (len)
			s[--len] = '/';

		kn = kn->parent;
	}

	return 0;
}

static int bsfs_getlink(struct inode *inode, char *path)
{
	struct bsfs_node *kn = inode->i_private;
	struct bsfs_node *parent = kn->parent;
	struct bsfs_node *target = kn->symlink.target_kn;
	int error;

	mutex_lock(&bsfs_mutex);
	error = bsfs_get_target_path(parent, target, path);
	mutex_unlock(&bsfs_mutex);

	return error;
}

static const char *bsfs_iop_get_link(struct dentry *dentry,
			struct inode *inode, struct delayed_call *done)
{
	char *body;
	int error;

	if (!dentry)
		return ERR_PTR(-ECHILD);
	body = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!body)
		return ERR_PTR(-ENOMEM);
	error = bsfs_getlink(inode, body);
	if (unlikely(error < 0)) {
		kfree(body);
		return ERR_PTR(error);
	}
	set_delayed_call(done, kfree_link, body);
	return body;
}

const struct inode_operations bsfs_symlink_iops = {
	.listxattr	= bsfs_iop_listxattr,
	.get_link	= bsfs_iop_get_link,
	.setattr	= bsfs_iop_setattr,
	.getattr	= bsfs_iop_getattr,
	.permission	= bsfs_iop_permission,
};
