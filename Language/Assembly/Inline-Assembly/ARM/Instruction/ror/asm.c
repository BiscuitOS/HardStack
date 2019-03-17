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
 * ROR Rotate right
 *
 * Syntax
 * 
 * RX, ROR #n 
 * or
 * RX, ROR Rn
 */

int debug_ror(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x1;

	/* MOV R0, R1, ROR #1 --> R0 = R1 >> 1 */
	__asm__ volatile ("mov %0, %1, ror #1"
		: "=r" (a0) : "r" (a1));

	printk("%#lx >> 1 = %#lx\n", a1, a0);

	return 0;
}
device_initcall(debug_ror);
