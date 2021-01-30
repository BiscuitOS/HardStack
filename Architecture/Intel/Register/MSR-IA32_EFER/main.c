/*
 * Intel IA32_EFER MSR
 *
 *  Extended Feature Enable Register (UA32_EFER MSR)
 *
 *  63                                         12     9 8 7                1 0
 *  +--------------------------------------------+-+-+-+-+------------------+-+
 *  |                                            | | | | |                  | |
 *  |                                            | | | | |                  | |
 *  |                                            | | | | |                  | |
 *  +--------------------------------------------+-+-+-+-+------------------+-+
 *                                                | |   |                    |
 *                                                | |   |                    |
 *                                                | |   |                    |
 *                                                | |   |                    |
 *  Execute Disable Bit Enable -------------------o |   |                    |
 *  IA-32e Mode Active -----------------------------o   |                    |
 *                                                      |                    |   
 *  IA-32e Mode Enable ---------------------------------o                    |
 *  SYSCALL Enable ----------------------------------------------------------o
 *
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
#include <asm/msr.h>
#include <asm/msr-index.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	u64 val;

	/* Read IA32_EFER MSR */
	rdmsrl(MSR_EFER, val);

	/* Long mode enable active (read-onlu) */
	if (val & EFER_LMA)
		printk("IA32 Long mode enable active.\n");
	else
		printk("IA32 Long mode enable inactive.\n");

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
