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

#include "internal.h"

static struct dentry *proc_lookupfd_bs(struct inode *dir,
				struct dentry *dentry, unsigned int flags)
{
	BS_DUP();
	return NULL;
}

int proc_fd_permission_bs(struct inode *inode, int mask)
{
	BS_DUP();
	return 0;
}

static int proc_readfd_bs(struct file *file, struct dir_context *ctx)
{
	BS_DUP();
	return 0;
}

const struct inode_operations proc_fd_inode_operations_bs = {
	.lookup		= proc_lookupfd_bs,
	.permission	= proc_fd_permission_bs,
	.setattr	= proc_setattr_bs,
};

const struct file_operations proc_fd_operations_bs = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_readfd_bs,
	.llseek		= generic_file_llseek,
};
