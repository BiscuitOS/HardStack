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
 * LSR Logical Shift right
 *
 * Syntax
 * 
 * RX, LSR #n 
 * or
 * RX, LSR Rn
 */

int debug_lsr(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x8;

	/* MOV R0, R1, LSR #3 --> R0 = R1 >> 3 */
	__asm__ volatile ("mov %0, %1, lsr #3"
		: "=r" (a0) : "r" (a1));

	printk("%#lx >> 3 = %#lx\n", a1, a0);

	return 0;
}
device_initcall(debug_lsr);
