/*
 * debugfs filesytem -- inode
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include "internal.h"

struct debugfs_mount_opts {
	kuid_t uid;
	kgid_t gid;
	umode_t mode;
};

struct debugfs_fs_info {
	struct debugfs_mount_opts mount_opts;
};

static bool debugfs_registered_bs;

static int debugfs_remount_bs(struct super_block *sb, int *flags, char *data)
{
	BS_DUP();
	return 0;
}

static int debugfs_show_options_bs(struct seq_file *m, struct dentry *root)
{
	BS_DUP();
	return 0;
}

static void debugfs_evict_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static void debugfs_release_dentry_bs(struct dentry *dentry)
{
	BS_DUP();
}

static struct vfsmount *debugfs_automount_bs(struct path *path)
{
	BS_DUP();
	return NULL;
}

static const struct super_operations debugfs_super_operations_bs = {
	.statfs		= simple_statfs,
	.remount_fs	= debugfs_remount_bs,
	.show_options	= debugfs_show_options_bs,
	.evict_inode	= debugfs_evict_inode_bs,
};

static const struct dentry_operations debugfs_dops_bs = {
	.d_delete	= always_delete_dentry,
	.d_release	= debugfs_release_dentry_bs,
	.d_automount	= debugfs_automount_bs,
};

static int debugfs_parse_options_bs(char *data, 
				struct debugfs_mount_opts *opts)
{
	char *p;

	while ((p = strsep(&data, ",")) != NULL) {
		BS_DUP();
	}
	return 0;
}

static int debugfs_apply_options_bs(struct super_block *sb)
{
	struct debugfs_fs_info *fsi = sb->s_fs_info;
	struct inode *inode = d_inode(sb->s_root);
	struct debugfs_mount_opts *opts = &fsi->mount_opts;

	inode->i_mode &= ~S_IALLUGO;
	inode->i_mode |= opts->mode;

	inode->i_uid = opts->uid;
	inode->i_gid = opts->gid;

	return 0;
}

static int debug_fill_super_bs(struct super_block *sb, void *data, int silent)
{
	static const struct tree_descr debug_files[] = {{""}};
	struct debugfs_fs_info *fsi;
	int err;

	fsi = kzalloc(sizeof(struct debugfs_fs_info), GFP_KERNEL);
	sb->s_fs_info = fsi;
	if (!fsi) {
		err = -ENOMEM;
		goto fail;
	}

	err = debugfs_parse_options_bs(data, &fsi->mount_opts);
	if (err)
		goto fail;

	err = simple_fill_super(sb, DEBUGFS_MAGIC_BS, debug_files);
	if (err)
		goto fail;

	sb->s_op = &debugfs_super_operations_bs;
	sb->s_d_op = &debugfs_dops_bs;

	debugfs_apply_options_bs(sb);

	return 0;

fail:
	kfree(fsi);
	sb->s_fs_info = NULL;
	return err;
}

static struct dentry *debug_mount_bs(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, debug_fill_super_bs);
}

static struct file_system_type debug_fs_type_bs = {
	.owner		= THIS_MODULE,
	.name		= "debugfs_bs",
	.mount		= debug_mount_bs,
	.kill_sb	= kill_litter_super,
};

static int __init debugfs_init_bs(void)
{
	int retval;

	retval = sysfs_create_mount_point(kernel_kobj, "debug_bs");
	if (retval)
		return retval;

	retval = register_filesystem(&debug_fs_type_bs);
	if (retval)
		sysfs_remove_mount_point(kernel_kobj, "debug_bs");
	else
		debugfs_registered_bs = true;

	return retval;
}
fs_initcall(debugfs_init_bs);
