/*
 * affs filesytem
 *
 * (C) 2020.04.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "internal.h"

static struct kmem_cache *affs_inode_cachep_bs;

static void init_once_bs(void *foo)
{
	struct affs_inode_info *ei = (struct affs_inode_info *)foo;

	sema_init(&ei->i_link_lock, 1);
	sema_init(&ei->i_ext_lock, 1);
	inode_init_once(&ei->vfs_inode);
}

static int __init init_inodecache_bs(void)
{
	affs_inode_cachep_bs = kmem_cache_create("affs_inode_cache_bs",
					sizeof(struct affs_inode_info),
					0,
					(SLAB_RECLAIM_ACCOUNT |
					SLAB_MEM_SPREAD | SLAB_ACCOUNT),
					init_once_bs);
	if (affs_inode_cachep_bs == NULL)
		return -ENOMEM;
	return 0;
}

static void destroy_inodecache_bs(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(affs_inode_cachep_bs);
}

static struct dentry *affs_mount_bs(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	BS_DUP();
	return NULL;
}

static void affs_kill_sb_bs(struct super_block *sb)
{
	BS_DUP();
}

static struct file_system_type affs_fs_type_bs = {
	.owner		= THIS_MODULE,
	.name		= "affs_bs",
	.mount		= affs_mount_bs,
	.kill_sb	= affs_kill_sb_bs,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init init_affs_fs_bs(void)
{
	int err = init_inodecache_bs();

	if (err)
		goto out1;
	err = register_filesystem(&affs_fs_type_bs);
	if (err)
		goto out;
	return 0;
out:
	destroy_inodecache_bs();
out1:
	return err;
}

static void __exit exit_affs_fs_bs(void)
{
	unregister_filesystem(&affs_fs_type_bs);
	destroy_inodecache_bs();
}

module_init(init_affs_fs_bs);
module_exit(exit_affs_fs_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("affs_bs filesystem");
