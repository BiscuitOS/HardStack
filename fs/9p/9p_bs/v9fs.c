/*
 * v9fs filesytem
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
#include <linux/kobject.h>

#include "internal.h"

struct kmem_cache *v9fs_inode_cache_bs;
static struct kobject *v9fs_kobj_bs;

static void v9fs_inode_init_once_bs(void *foo)
{
	struct v9fs_inode *v9inode = (struct v9fs_inode *)foo;

	memset(&v9inode->qid, 0, sizeof(v9inode->qid));
	inode_init_once(&v9inode->vfs_inode);
}

/*
 * v9fs_init_inode_cache_bs - initialize a cache for 9P
 * Return 0 on success.
 */
static int v9fs_init_inode_cache_bs(void)
{
	v9fs_inode_cache_bs = kmem_cache_create("v9fs_inode_cache_bs",
					sizeof(struct v9fs_inode),
					0,
					(SLAB_RECLAIM_ACCOUNT | 
					SLAB_MEM_SPREAD | SLAB_ACCOUNT),
					v9fs_inode_init_once_bs);
	if (!v9fs_inode_cache_bs)
		return -ENOMEM;

	return 0;
}

/*
 * v9fs_destroy_inode_cache - destroy the cache of 9P inode.
 */
static void v9fs_destroy_inode_cache_bs(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(v9fs_inode_cache_bs);
}

static int v9fs_cache_register_bs(void)
{
	int ret;

	ret = v9fs_init_inode_cache_bs();
	if (ret < 0)
		return ret;

	return ret;
}

static void v9fs_cache_unregister_bs(void)
{
	v9fs_destroy_inode_cache_bs();
}

static struct attribute *v9fs_attrs_bs[] = {
	NULL,
};

static struct attribute_group v9fs_attr_group_bs = {
	.attrs = v9fs_attrs_bs,
};

/*
 * v9fs_sysfs_cleanup_bs - Unregister the v9fs sysfs interface
 */
static void v9fs_sysfs_cleanup_bs(void)
{
	sysfs_remove_group(v9fs_kobj_bs, &v9fs_attr_group_bs);
	kobject_put(v9fs_kobj_bs);
}

/*
 * v9fs_sysfs_init_bs - Initialize the v9fs sysfs interface
 */
static int __init v9fs_sysfs_init_bs(void)
{
	v9fs_kobj_bs = kobject_create_and_add("9p_bs", fs_kobj); 
	if (!v9fs_kobj_bs)
		return -ENOMEM;

	if (sysfs_create_group(v9fs_kobj_bs, &v9fs_attr_group_bs)) {
		kobject_put(v9fs_kobj_bs);
		return -ENOMEM;
	}

	return 0;
}

/*
 * init_v9fs - Initialize module
 */
static int __init init_v9fs_bs(void)
{
	int err;

	printk("Installing v9fs 9p2000 file system support\n");
	/* TODO: Setup list of register trasnport modules */

	err = v9fs_cache_register_bs();
	if (err < 0) {
		printk("Failed to register v9fs for caching\n");
		return err;
	}

	err = v9fs_sysfs_init_bs();
	if (err < 0) {
		printk("Failed to register with sysfs\n");
		goto out_cache;
	}

	err = register_filesystem(&v9fs_fs_type_bs);
	if (err < 0) {
		printk("Failed to register filesystem\n");
		goto out_sysfs_cleanup;
	}

	printk("Finish....\n");
	return 0;

out_sysfs_cleanup:
	v9fs_sysfs_cleanup_bs();

out_cache:
	v9fs_cache_unregister_bs();

	return err;
}

static void __exit exit_v9fs_bs(void)
{
	v9fs_sysfs_cleanup_bs();
	v9fs_cache_unregister_bs();
	unregister_filesystem(&v9fs_fs_type_bs);
}

module_init(init_v9fs_bs);
module_exit(exit_v9fs_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("v9fs_bs filesystem");
