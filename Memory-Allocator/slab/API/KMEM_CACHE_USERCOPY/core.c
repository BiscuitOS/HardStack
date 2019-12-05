/*
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

static struct kmem_cache *BiscuitOS_cache __read_mostly;

/* private data */
struct Demo_node
{
	int index;
	int flags;
	int bitmap;
	int offset;
	int max;
};

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct Demo_node *node;
	struct Demo_node *node1;

	BiscuitOS_cache = KMEM_CACHE_USERCOPY(Demo_node,
		SLAB_RECLAIM_ACCOUNT|SLAB_PANIC|SLAB_MEM_SPREAD|SLAB_ACCOUNT,
		flags);

	node = kmem_cache_alloc(BiscuitOS_cache, GFP_KERNEL);
	if (!node) {
		printk("Faild to allocate cache\n");
		return -ENOMEM;
	}
	node1 = kmem_cache_alloc(BiscuitOS_cache, GFP_KERNEL);
	if (!node1) {
		printk("Faild to allocate cache1\n");
		return -ENOMEM;
	}
	
	/* indicate size for cache */
	printk("SIZE: %ld\n", (unsigned long)node1 - (unsigned long)node);

	kmem_cache_free(BiscuitOS_cache, node1);
	kmem_cache_free(BiscuitOS_cache, node);

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Device driver");
