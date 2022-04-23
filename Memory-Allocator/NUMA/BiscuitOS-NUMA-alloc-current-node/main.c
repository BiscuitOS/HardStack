/*
 * NUMA: allocate page from Current NODE
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
	struct zone *zone;
	char *base;

	/* alloc page */
	page = alloc_page(GFP_KERNEL | __GFP_THISNODE);
	if (!page) {
		printk("NODE %d not free page.\n", numa_node_id());
		return -EINVAL;
	}

	/* usage */
	base = page_address(page);
	sprintf(base, "BiscuitOS NODE %d", numa_node_id());
	printk("%s\n", base);

	/* Zone information */
	zone = page_zone(page);
	printk("Page %#lx From NODE-%d\n", page_to_pfn(page), 
					zone->zone_pgdat->node_id);

	/* free */
	__free_page(page);

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
MODULE_DESCRIPTION("BiscuitOS NUMA");
