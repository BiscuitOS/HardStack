/*
 * CRASH: ptov
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
#include <linux/slab.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	void *address;

	address = kmalloc(8, GFP_KERNEL);
	printk("Virtual Address: %#lx\n", (unsigned long)address);

	/* panic for CRASH */
	panic("CRASH ptov");

	kfree(address);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void) {}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CRASH ptov on BiscuitOS");
