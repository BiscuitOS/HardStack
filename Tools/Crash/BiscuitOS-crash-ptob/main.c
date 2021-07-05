/*
 * CRASH: ptob
 *
 * (C) 2021.06.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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
	phys_addr_t phys;
	unsigned long pfn;

	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* Translation ptob */
	pfn = page_to_pfn(page);
	phys = PFN_PHYS(pfn);

	printk("PFN %#lx PHYS %#lx\n", pfn, (unsigned long)phys);

	/* panic for CRASH */
	panic("CRASH ptob");

	__free_page(page);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) {}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CRASH ptob on BiscuitOS");
