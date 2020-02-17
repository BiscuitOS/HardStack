/*
 * Kmap
 *
 * (C) 2020.02.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/highmem.h>

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct page *page = alloc_page(GFP_KERNEL);
	unsigned long pfn = page_to_pfn(page);
	char *mpage;

	if (!page) {
		printk("BUG(): alloc_page failed\n");
		return -ENOMEM;
	}

	mpage = kmap_atomic_pfn(pfn);
	printk("mpages address: %#lx\n", (unsigned long)mpage);
	sprintf(mpage, "BiscuitOS-%x\n", 0x91);
	printk("Output: %s\n", mpage);

	kunmap_atomic(mpage);
	__free_page(page);

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
MODULE_DESCRIPTION("Kmap Memory Allocator");
