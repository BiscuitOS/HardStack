/*
 * CACHE Instruction: CLFLUSH - Flush CACHE Line for virtual memory range
 *
 * (C) 2023.02.17 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/cacheflush.h>
#include <linux/highmem.h>

static int __init BiscuitOS_init(void)
{
	struct page *page;
	void *mem;

	/* Alloc physical page */
	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("System Error: No free Memory!\n");
		return -ENOMEM;
	}

	/* Temporary mapping */
	mem = kmap_local_page(page);
	if (!mem) {
		printk("KMAP Temporary mapping failed.\n");
		__free_page(page);
		return -EINVAL;
	}

	/* use */
	sprintf((char *)mem, "Hello BiscuitOS");
	printk("%s: Vaddr %#lx Phys %#lx\n", (char *)mem,
				(unsigned long)mem, page_to_pfn(page));

	/* Flush CACHE */
	clflush_cache_range(mem, PAGE_SIZE);

	/* Unmapping and reclaim */
	kunmap_local(mem);
	__free_page(page);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CLFlush CACHE on BiscuitOS");
