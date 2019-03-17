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
 * SBC (Subtract with Carry) subtracts the value of its second operand 
 * and the value of NOT(Carry flag) from the value of its first operand. 
 * The first operand comes from a register. The second operand can be 
 * either an immediate value or a value from a register, and can be 
 * shifted before the subtraction. Use SBC to synthesize multi-word 
 * subtraction. SBC can optionally update the condition code flags, based
 * on the result.
 *
 * Syntax
 *   SBC{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 *
 * Usage
 *
 * - If register pairs R0,R1 and R2,R3 hold 64-bit values (R0 and R2 hold
 *   the least significant words), the following instructions leave the 
 *   64-bit difference in R4,R5:
 *       SUBS R4,R0,R2
 *       SBC R5,R1,R3
 */

int debug_sbc(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0xa;
	unsigned long a2 = 0x2;

	/* SBC R0, R1, R2 --> R0 = R1 - R2 - !carry */
	__asm__ volatile ("sbc %0, %1, %2"
		: "=r" (a0) : "r" (a1), "r" (a2));

	printk("%#lx - %#lx = %#lx\n", a2, a1, a0);

	return 0;
}
device_initcall(debug_sbc);
