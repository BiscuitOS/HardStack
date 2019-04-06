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
 * instruction{CC}
 *
 *   If R1 - R2 < 0
 *      THEN
 *          XXCC
 *      ELSE
 *   FI
 *   The instruction execute when CC or LO C clear Lower (unsigned < )
 */

static int debug_CC(void)
{
	unsigned long R0 = 4;
	unsigned long R1 = 3;
	unsigned long R2 = 0;

	__asm__ volatile ("cmp %1, %2\n\r"
			  "movcc %0, #1"
			  : "=r" (R2) : "r" (R0), "r" (R1));

	/* If R0 < R1 ---> Execute movcc */
	printk("%#lx - %#lx ---> %#lx\n", R0, R1, R2);

	return 0;
}
device_initcall(debug_CC);
