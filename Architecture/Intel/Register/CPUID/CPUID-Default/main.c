/*
 * CPUID
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
/* CPUID */
#include <asm/processor.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned int eax, ebx, ecx, edx;

	eax = ebx = ecx = edx = 0;

	/* CPUID 01H */
	cpuid(0x01, &eax, &ebx, &ecx, &edx);

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
MODULE_DESCRIPTION("CPUID on BiscuitOS");
