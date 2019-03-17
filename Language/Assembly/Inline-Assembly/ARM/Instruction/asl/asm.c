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
 * Arithmetic Shift Left
 *
 * Syntax
 * 
 * RX, ASL #n 
 * or
 * RX, ASL Rn
 */

int debug_asl(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x1;

	/* MOV R0, R1, ASL #3 --> R0 = R1 << 3 */
	__asm__ volatile ("mov %0, %1, asl #3"
		: "=r" (a0) : "r" (a1));

	printk("%#lx << 3 = %#lx\n", a1, a0);

	return 0;
}
device_initcall(debug_asl);
