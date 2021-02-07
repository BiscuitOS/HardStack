/*
 * Intel CR3 Register
 *
 * CR3 With 32-Bit Paging
 *
 *  63         32 31                           12 11         5 4 3 2      0
 * +-------------+-------------------------------+------------+-+-+--------+
 * |             |                               |            |P|P|        |
 * |   Ignored   | Physical Address of 4KiB Page |   Ignore   |C|W| Ignore |
 * |             |                               |            |D|T|        |
 * +-------------+-------------------------------+------------+-+-+--------+
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* Header */
#include <asm/paravirt.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long cr3;

	cr3 = __read_cr3();
	printk("CR3 Register %#lx\n", cr3);

	/* PWT */ 
	if (cr3 & X86_CR3_PWT)
		printk("  CR3 PWT (Page Write Through) Enable.\n");
	else
		printk("  CR3 PWT (Page Write Through) Disable.\n");

	/* PCD */ 
	if (cr3 & X86_CR3_PCD)
		printk("  CR3 PCD (Page Cache Disable) Enable.\n");
	else
		printk("  CR3 PCD (Page Cache Disable) Disable.\n");

	/* Physical Address */
	printk("  Physical Address from CR3: %#lx\n", cr3 & PAGE_MASK);

	printk("Hello modules on BiscuitOS\n");

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
MODULE_DESCRIPTION("Intel Register on BiscuitOS");
