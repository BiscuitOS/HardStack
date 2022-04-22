/*
 * MM-ERROR: Confuse GFP flags on SLAB/SLUB/SLOB Allocator
 *
 * (C) 2022.04.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#ifndef CONFIG_DEBUG_VM
#error "Kernel must enable CONFIG_DEBUG_VM"
#endif

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	char *buffer;

	buffer = kmalloc(256, __GFP_DMA | __GFP_DMA32);
	if (!buffer) {
		printk("Alocate Memory Failed.\n");
		return -ENOMEM;
	}

	kfree(buffer);

	printk("Hello modules on BiscuitOS\n");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("MM-ERROR: Confues GFP flags");
