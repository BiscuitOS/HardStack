/*
 * sysfs filesytem -- mount
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernfs.h>
#include <linux/kobject_ns.h>

#include "internal.h"

static struct kernfs_root *sysfs_root_bs;
struct kernfs_node *sysfs_root_kn_bs;

static struct dentry *sysfs_mount_bs(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	struct dentry *root;
	void *ns;
	bool new_sb = false;

	if (!(flags & SB_KERNMOUNT)) {
		if (!kobj_ns_current_may_mount(KOBJ_NS_TYPE_NET))
			return ERR_PTR(-EPERM);
	}

	ns = kobj_ns_grab_current(KOBJ_NS_TYPE_NET);
	root = kernfs_mount_ns(fs_type, flags, sysfs_root_bs,
			SYSFS_MAGIC_BS, &new_sb, ns);
	if (!new_sb)
		kobj_ns_drop(KOBJ_NS_TYPE_NET, ns);
	else if (!IS_ERR(root))
		root->d_sb->s_iflags |= SB_I_USERNS_VISIBLE;

	return root;
}

static void sysfs_kill_sb_bs(struct super_block *sb)
{
	void *ns = (void *)kernfs_super_ns(sb);

	kernfs_kill_sb(sb);
	kobj_ns_drop(KOBJ_NS_TYPE_NET, ns);
}

static struct file_system_type sysfs_fs_type_bs = {
	.name		= "sysfs_bs",
	.mount		= sysfs_mount_bs,
	.kill_sb	= sysfs_kill_sb_bs,
	.fs_flags	= FS_USERNS_MOUNT,
};

static __init int sysfs_init_bs(void)
{
	int err;

	sysfs_root_bs = kernfs_create_root(NULL,
					   KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK,
					   NULL);
	if (IS_ERR(sysfs_root_bs))
		return PTR_ERR(sysfs_root_bs);

	sysfs_root_kn_bs = sysfs_root_bs->kn;

	err = register_filesystem(&sysfs_fs_type_bs);
	if (err) {
		kernfs_destroy_root(sysfs_root_bs);
		return err;
	}

	return 0;
}
fs_initcall(sysfs_init_bs);
