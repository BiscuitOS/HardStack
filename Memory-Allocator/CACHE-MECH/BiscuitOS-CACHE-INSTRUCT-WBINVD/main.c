/*
 * CACHE Instruction: WBINVD - WriteBack and Invalid CACHE
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
#include <asm/smp.h>

static int __init BiscuitOS_init(void)
{
	void *addr;

	/* alloc */
	addr = (void *)__get_free_page(GFP_KERNEL);
	if (!addr) {
		printk("System Error: No free memory on Buddy\n");
		return -ENOMEM;
	}

	/* Write Hit/Miss on CACHE Line */
	sprintf((char *)addr, "Hello BiscuitOS");

	/* WriteBack and flush CACHE */
	wbinvd_on_all_cpus();

	/* Read Exclusive CACHE Line */
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
MODULE_DESCRIPTION("WBINVD on BiscuitOS");
