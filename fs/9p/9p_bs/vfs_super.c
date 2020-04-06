/*
 * v9fs: vfs
 *
 * (C) 2020.04.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include "internal.h"

/*
 * v9fs_mount - mount a superblock
 * @fs_type: file system type
 * @flags: mount flags
 * @dev_name: device name that was mounted
 * @data: mount options
 */
static struct dentry *v9fs_mount_bs(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	BS_DUP();
	return NULL;
}

static void v9fs_kill_super_bs(struct super_block *s)
{
	BS_DUP();
}

struct file_system_type v9fs_fs_type_bs = {
	.name = "9p_bs",
	.mount = v9fs_mount_bs,
	.kill_sb = v9fs_kill_super_bs,
	.owner = THIS_MODULE,
	.fs_flags = FS_RENAME_DOES_D_MOVE,
};
MODULE_ALIAS_FS("9p_bs");
