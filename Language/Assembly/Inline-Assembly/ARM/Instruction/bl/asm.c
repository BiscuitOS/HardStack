/*
 * Arm inline-assembly
 *
 * (C) 2019.03.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * BL (Branch and Link) cause a branch to a target address, and provide
 * both conditional and unconditional changes to program flow. BL also 
 * stores a return address in the link register, R14 (also known as LR).
 *
 * Syntax
 *   B{L}{<cond>} <target_address>
 */

int debug_show(void)
{
	printk("Hello World\n");
	return 0;
}

static int debug_bl(void)
{
	__asm__ volatile ("bl debug_show");

	return 0;
}
device_initcall(debug_bl);
