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
 * SUB (Subtract) subtracts one value from a second value. The second value
 * comes from a register. The first value can be either an immediate value
 * or a value from a register, and can be shifted before the subtraction.
 * SUB can optionally update the condition code flags, based on the result.
 *
 * Syntax
 *   SUB{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 *
 * Usage
 *
 * - Use SUB to subtract one value from another. To decrement a register
 *   value (in Ri ) use:
 *       SUB Ri, Ri, #1
 * - SUBS is useful as a loop counter decrement, as the loop branch can 
 *   test the flags for the appropriate termination condition, without the
 *   need for a separate compare instruction:
 *       SUBS Ri, Ri, #1
 * - This both decrements the loop counter in Ri and checks whether it 
 *   has reached zero.
 * - You can use SUB , with the PC as its destination register and the S 
 *   bit set, to return from interrupts and various other types of 
 *   exception. See Exceptions on page A2-16 for more details.
 */

int debug_sub(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x8;
	unsigned long a2 = 0x1;

	/* SUB R0, R1, R2 --> R0 = R1 - R2*/
	__asm__ volatile ("sub %0, %1, %2"
		: "=r" (a0) : "r" (a1), "r" (a2));

	printk("%#lx - %#lx = %#lx\n", a1, a2, a0);

	/* SUB R0, R1, R2, LSL #3 --> R0 = R1 - (R2 << 3) */
	__asm__ volatile ("sub %0, %1, %2, lsl #3"
		: "=r" (a0) : "r" (a1), "r" (a2));

	printk("%#lx - (%#lx << 3) = %#lx\n", a1, a2, a0);

	return 0;
}
device_initcall(debug_sub);
