/*
 * CACHE Instruction: MOVNTI - Store Doubleword Using Non-Temporal Hint
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
#include <linux/slab.h>

static int __init BiscuitOS_init(void)
{
	void *addr;
	void *mem;

	/* alloc */
	addr = (void *)__get_free_page(GFP_KERNEL);
	if (!addr) {
		printk("System Error: No free memory on Buddy\n");
		return -ENOMEM;
	}
	mem = kmalloc(PAGE_SIZE / 8, GFP_KERNEL);
	if (!mem) {
		printk("System Error: No free memory on Slab\n");
		free_page((unsigned long)addr);
		return -ENOMEM;
	}

	sprintf((char *)addr, "Hello BiscuitOS");

	/* COPY and Flush CACHE */
	__memcpy_flushcache(mem, addr, PAGE_SIZE / 8);

	printk("%s\n", (char *)mem);

	/* free */
	kfree(mem);
	free_page((unsigned long)addr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("MOVNTI on BiscuitOS");
