/*
 * ZONE: allocate page from ZONE_DMA
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
	page = alloc_page(__GFP_DMA);
	if (!page) {
		printk("__GFP_DMA not free page.\n");
		return -EINVAL;
	}

	/* usage */
	base = page_address(page);
	sprintf(base, "BiscuitOS ZONE_%s", "DMA");
	printk("%s\n", base);

	/* Zone information */
	zone = page_zone(page);
	printk("Page %#lx From NODE-%d ZONE %s\n", page_to_pfn(page),
			zone->zone_pgdat->node_id, zone->name);

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
MODULE_DESCRIPTION("BiscuitOS ZONE");
