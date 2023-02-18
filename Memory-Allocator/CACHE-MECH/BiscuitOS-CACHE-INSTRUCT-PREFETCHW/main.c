/*
 * CACHE Instruction: PREFETCHW - Prefetch Data into CACHE
 *                                in Anticipation of a Write
 *
 * (C) 2023.02.18 BuddyZhang1 <buddy.zhang@aliyun.com>
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
	void *addr;

	/* alloc */
	addr = (void *)__get_free_page(GFP_KERNEL);
	if (!addr) {
		printk("System Error: No free memory!\n");
		return -ENOMEM;
	}

	/* Prefetch Data into CACHE */
	prefetchw(addr);

	sprintf((char *)addr, "Hello BiscuitOS");
	printk("%s\n", (char *)addr);

	/* free */
	free_page((unsigned long)addr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Prefetchw Data into CACHE on BiscuitOS");
