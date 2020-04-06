/*
 * adfs filesytem
 *
 * (C) 2020.04.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>

#include "internal.h"

static struct kmem_cache *adfs_inode_cachep_bs;

static void init_once_bs(void *foo)
{
	struct adfs_inode_info *ei = (struct adfs_inode_info *)foo;

	inode_init_once(&ei->vfs_inode);
}

static int __init init_inodecache_bs(void)
{
	adfs_inode_cachep_bs = kmem_cache_create("adfs_inode_cache_bs",
					sizeof(struct adfs_inode_info),
					0,
					(SLAB_RECLAIM_ACCOUNT | 
					SLAB_MEM_SPREAD | SLAB_ACCOUNT),
					init_once_bs);
	if (adfs_inode_cachep_bs == NULL)
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
	kmem_cache_destroy(adfs_inode_cachep_bs);
}

static struct dentry *adfs_mount_bs(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	BS_DUP();
	return NULL;
}

static struct file_system_type adfs_fs_type_bs = {
	.owner		= THIS_MODULE,
	.name		= "adfs_bs",
	.mount		= adfs_mount_bs,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};
MODULE_ALIAS_FS("adfs_bs");

static int __init init_adfs_fs_bs(void)
{
	int err = init_inodecache_bs();

	if (err)
		goto out1;

	err = register_filesystem(&adfs_fs_type_bs);
	if (err)
		goto out;
	return 0;
out:
	destroy_inodecache_bs();
out1:
	return err;
}

static void __exit exit_adfs_fs_bs(void)
{

}

module_init(init_adfs_fs_bs);
module_exit(exit_adfs_fs_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("adfs_bs filesystem");
