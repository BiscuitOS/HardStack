// SPDX-License-Identifier: GPL-2.0
/*
 * Linear Mapping: SLAB
 *
 * (C) 2023.10.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

static int __init BiscuitOS_init(void)
{
	struct kmem_cache *kcache;	
	void *addr;

	kcache = kmem_cache_create("BiscuitOS", 64, 0, 0, NULL);	
	if (!kcache)
		return -ENOMEM;

	addr = kmem_cache_alloc(kcache, GFP_KERNEL);
	if (!addr) {
		kmem_cache_destroy(kcache);
		return -ENOMEM;
	}

	/* Use memory */
	sprintf((char *)addr, "Hello BiscuitOS");
	printk("LM %#lx => %s\n", (unsigned long)addr, (char *)addr);

	/* Reclaim */
	kmem_cache_free(kcache, addr);
	kmem_cache_destroy(kcache);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging Project");
