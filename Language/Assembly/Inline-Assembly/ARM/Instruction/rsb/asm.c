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
 * RSB (Reverse Subtract) subtracts a value from a second value. The first
 * value comes from a register. The second value can be either an immediate
 * value or a value from a register, and can be shifted before the 
 * subtraction. This is the reverse of the normal order of operands in ARM
 * assembler language. RSB can optionally update the condition code flags,
 * based on the result.
 *
 * Syntax
 *   RSB{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 *
 * Usage
 *
 * - The following instruction stores the negation (twos complement) of 
 *   Rx in Rd :
 *       RSB Rd, Rx, #0
 * - You can perform constant multiplication (of Rx ) by 2 n –1 (into Rd)
 *   with:
 *       RSB Rd, Rx, Rx, LSL #n
 */

static int debug_rsb(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0x2;
	unsigned long a2 = 0xa;

	/* RSB R0, R1, R2 --> R0 = R2 - R1 */
	__asm__ volatile ("rsb %0, %1, %2"
		: "=r" (a0) : "r" (a1), "r" (a2));

	printk("%#lx - %#lx = %#lx\n", a2, a1, a0);

	/* RSB R0, R1, R2, LSL #3 ---> R0 = (R2 << 3) - R1 */
	__asm__ volatile ("rsb %0, %1, %2, lsl #3"
		: "=r" (a0) : "r" (a1), "r" (a2));
	printk("(%#lx << 3) - %#lx = %#lx\n", a2, a1, a0);

	return 0;
}
device_initcall(debug_rsb);
