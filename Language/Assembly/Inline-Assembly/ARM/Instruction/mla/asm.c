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
 * MLA (Multiply Accumulate) multiplies two signed or unsigned 32-bit 
 * values, and adds a third 32-bit value. The least significant 32 bits
 * of the result are written to the destination register. MLA can 
 * optionally update the condition code flags, based on the result.
 *
 * Syntax
 *   MLA{<cond>}{S} <Rd>, <Rm>, <Rs>, <Rn>
 */

static int debug_mla(void)
{
	unsigned long R1 = 0x2;
	unsigned long R2 = 0x3;
	unsigned long R3 = 0x4;
	unsigned long total;

	/* MLA R1, R2, R3, R4 ---> R1 = (R2 * R3) + R4 */
	__asm__ volatile ("mla %0, %1, %2, %3"
			: "=r" (total) : "r" (R1), "r" (R2), "r" (R3));

	printk("(%#lx * %#lx) + %#lx = %#lx\n", R1, R2, R3, total);

	return 0;
}
device_initcall(debug_mla);
