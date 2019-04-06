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
 * MUL (Multiply) multiplies two signed or unsigned 32-bit values. The
 * least significant 32 bits of the result are written to the destination
 * register. MUL can optionally update the condition code flags, based on
 * the result.
 *
 * Syntax
 *   MUL{<cond>}{S} <Rd>, <Rm>, <Rs>
 */

static int debug_mul(void)
{
	unsigned long R1 = 0x2;
	unsigned long R2 = 0x3;
	unsigned long total;

	/* MUL R1, R2, R3 ---> R1 = R2 * R3 */
	__asm__ volatile ("mul %0, %1, %2"
			: "=r" (total) : "r" (R1), "r" (R2));

	printk("%#lx * %#lx = %#lx\n", R1, R2, total);

	return 0;
}
device_initcall(debug_mul);
