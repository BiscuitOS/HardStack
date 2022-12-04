/*
 * PAGE: allocate a physical page on kernel
 *
 * (C) 2022.04.22 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct page *page;
	char *base;

	/* alloc page */
	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("GFP_KERNEL not free page.\n");
		return -EINVAL;
	}

	/* usage */
	base = page_address(page);
	sprintf(base, "BiscuitOS ZONE_%s", "Normal");
	printk("Page %#lx\n PFN %#lx\n PHY %#lx\n KVA %#lx\n VAL %s\n",
		(unsigned long)page, page_to_pfn(page),
		page_to_pfn(page) << PAGE_SHIFT,
		(unsigned long)base, base);

	/* free */
	__free_page(page);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGE");
