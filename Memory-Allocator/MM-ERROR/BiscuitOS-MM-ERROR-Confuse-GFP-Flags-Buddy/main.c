/*
 * MM-ERROR: Confuse GFP flags on Buddy Allocator
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
#include <linux/mm.h>

#ifndef CONFIG_DEBUG_VM
#error "Kernel must enable CONFIG_DEBUG_VM"
#endif

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *pages;

	/* Allocate memory */
	pages = alloc_pages(__GFP_DMA32 | __GFP_DMA, 2);
	if (!pages) {
		printk("ERROR: Allocate page failed.\n");
		return -ENOMEM;
	}

	printk("Hello modules on BiscuitOS\n");
	__free_pages(pages, 2);

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
