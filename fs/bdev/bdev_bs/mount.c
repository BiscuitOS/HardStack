/*
 * bdev filesytem -- mount
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/buffer_head.h>
#include <linux/module.h>

#include "internal.h"

struct bdev_inode {
	struct block_device bdev;
	struct inode vfs_inode;
};

static struct kmem_cache *bdev_cachep_bs __read_mostly;
static __cacheline_aligned_in_smp DEFINE_SPINLOCK(bdev_lock_bs);

static inline struct bdev_inode *BDEV_I_BS(struct inode *inode)
{
	return container_of(inode, struct bdev_inode, vfs_inode);
}

/*
 * pseudo-fs
 */

static struct inode *bdev_alloc_inode_bs(struct super_block *sb)
{
	struct bdev_inode *ei = kmem_cache_alloc(bdev_cachep_bs, GFP_KERNEL);

	if (!ei)
		return NULL;
	return &ei->vfs_inode;
}

static void bdev_i_callback_bs(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);
	struct bdev_inode *bdi = BDEV_I_BS(inode);

	kmem_cache_free(bdev_cachep_bs, bdi);
}

static void bdev_destroy_inode_bs(struct inode *inode)
{
	call_rcu(&inode->i_rcu, bdev_i_callback_bs);
}

static void bdev_evict_inode_bs(struct inode *inode)
{
	struct block_device *bdev = &BDEV_I_BS(inode)->bdev;

	truncate_inode_pages_final(&inode->i_data);
	invalidate_inode_buffers(inode);
	clear_inode(inode);
	spin_lock(&bdev_lock_bs);
	list_del_init(&bdev->bd_list);
	spin_unlock(&bdev_lock_bs);
	/* Detach inode from wb early as bdi_put() may free bdi->wb */
	inode_detach_wb(inode);
	if (bdev->bd_bdi != &noop_backing_dev_info) {
		bdi_put(bdev->bd_bdi);
		bdev->bd_bdi = &noop_backing_dev_info;
	}
}

static void init_once_bs(void *foo)
{
	struct bdev_inode *ei = (struct bdev_inode *)foo;
	struct block_device *bdev = &ei->bdev;

	memset(bdev, 0, sizeof(*bdev));
	mutex_init(&bdev->bd_mutex);
	INIT_LIST_HEAD(&bdev->bd_list);
	INIT_LIST_HEAD(&bdev->bd_holder_disks);
	bdev->bd_bdi = &noop_backing_dev_info;
	inode_init_once(&ei->vfs_inode);
	/* Initialize mutex for freeze */
	mutex_init(&bdev->bd_fsfreeze_mutex);
}

static const struct super_operations bdev_sops_bs = {
	.statfs		= simple_statfs,
	.alloc_inode	= bdev_alloc_inode_bs,
	.destroy_inode	= bdev_destroy_inode_bs,
	.drop_inode	= generic_delete_inode,
	.evict_inode	= bdev_evict_inode_bs,
};

static struct dentry *bd_mount_bs(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	struct dentry *dent;

	dent = mount_pseudo(fs_type, "bdev_bs:", &bdev_sops_bs, 
						NULL, BDEVFS_MAGIC_BS);
	if (!IS_ERR(dent))
		dent->d_sb->s_iflags |= SB_I_CGROUPWB;
	return dent;
}

static struct file_system_type bd_type_bs = {
	.name		= "bdev_bs",
	.mount		= bd_mount_bs,
	.kill_sb	= kill_anon_super,
	.fs_flags	= FS_USERNS_MOUNT,
};

static int __init bdev_cache_init_bs(void)
{
	int err;

	bdev_cachep_bs = kmem_cache_create("bdev_cache_bs",
					sizeof(struct bdev_inode),
					0,
					(SLAB_HWCACHE_ALIGN | 
					SLAB_RECLAIM_ACCOUNT |
					SLAB_MEM_SPREAD | SLAB_ACCOUNT |
					SLAB_PANIC),
					init_once_bs);
	err = register_filesystem(&bd_type_bs);
	if (err)
		panic("Cannot register bdev pseudo-fs");

	return 0;
}

static void __exit bdev_cache_exit_bs(void)
{
	unregister_filesystem(&bd_type_bs);
}

module_init(bdev_cache_init_bs);
module_exit(bdev_cache_exit_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("bdev filesystem");
