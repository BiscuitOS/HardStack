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
 * RSC (Reverse Subtract with Carry) subtracts one value from another, 
 * taking account of any borrow from a preceding less significant 
 * subtraction. The normal order of the operands is reversed, to allow 
 * subtraction from a shifted register value, or from an immediate value.
 * RSC can optionally update the condition code flags, based on the result.
 *
 * Syntax
 *   RSC{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 */

int debug_rsc(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x2;
	unsigned long a2 = 0xa;

	/* RSC R0, R1, R2 --> R0 = R2 - R1 - !carry */
	__asm__ volatile ("rsc %0, %1, %2"
		: "=r" (a0) : "r" (a1), "r" (a2));

	printk("%#lx - %#lx = %#lx\n", a2, a1, a0);

	return 0;
}
device_initcall(debug_rsc);
