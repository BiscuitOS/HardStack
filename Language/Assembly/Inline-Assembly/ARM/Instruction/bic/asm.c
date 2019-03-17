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
 * BIC (Bit Clear) performs a bitwise AND of one value with the complement 
 * of a second value. The first value comes from a register. The second 
 * value can be either an immediate value or a value from a register, and 
 * can be shifted before the BIC operation. BIC can optionally update the 
 *condition code flags, based on the result.
 *
 * Syntax
 *   BIC{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 */

int debug_bic(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0xFFFF;
	unsigned long a2 = 0xb;

	/* BIC R0, R1, R2 ---> R0 = R1 Clear bit on R2  */
	__asm__ volatile ("bic %0, %1, %2" :: 
		"r" (a0), "r" (a1), "r" (a2));

	printk("%#lx Clear %#lx = %#lx\n", a1, a2, a0);

	return 0;
}
device_initcall(debug_bic);
