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
 * BHI ---> >   ---> Big then with unsigned-
 *
 * Syntax
 *   BHI <branch>
 */

static int debug_bhi(void)
{
	unsigned long R1 = 0x2;
	unsigned long R2 = 0x1;
	unsigned long RET;

	/* BHI <branch> --> Big and jump to branck */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "bhi _big\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_big:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx > %#lx", R1, R2);
	else
		printk("%#lx < %#lx", R1, R2);

	return 0;
}
device_initcall(debug_bhi);
