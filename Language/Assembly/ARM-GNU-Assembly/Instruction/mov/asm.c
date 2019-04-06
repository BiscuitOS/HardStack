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
 * MOV (Move) writes a value to the destination register. The value can be
 * either an immediate value or a value from a register, and can be shifted
 * before the write.
 *
 * MOV can optionally update the condition code flags, based on the result.
 *
 * Syntax
 *
 *   MOV {<cond>} {S} <Rd>, <shifter_operand>
 *
 * Usage
 *
 * - Move a value from one register to another.
 * - Put a constant value into a register.
 * - Perform a shift without any other arithmetic or logical operation. Use 
 *   a left shift by n to multiply by 2^n .
 * - When the PC is the destination of the instruction, a branch occurs. The
 *   instruction:
 *           MOV PC, LR
 *   can therefore be used to return from a subroutine (see instructions B, 
 *   BL on page A4-10). In T variants of architecture 4 and in architecture 
 *   5 and above, the instruction BX LR must be used in place of MOV PC, LR, 
 *   as the BX instruction automatically switches back to Thumb state if 
 *   appropriate (but see also The T and J bits on page A2-15 for operation
 *   on non-T variants of ARM architecture version 5).
 * - When the PC is the destination of the instruction and the S bit is set,
 *   a branch occurs and the SPSR of the current mode is copied to the CPSR.
 *   This means that you can use a MOVS PC, LR instruction to return from 
 *   some types of exception (see Exceptions on page A2-16).
 */

static int debug_mov(void)
{
	unsigned long reg;
	unsigned long val = 0x3;

	/* NOP */
	__asm__ volatile ("mov r0, r0");

	/* Put a constant value into a register */
	__asm__ volatile ("mov r0, %0" :: "r" (val));

	/* Move a value from one register to another */
	__asm__ volatile ("mov %0, r0": "=r" (reg));
	printk("Register R0: %#lx\n", reg);

	/* Perform a shift without any other arithmetic or logical operation.
         * Use a left shift by n to multiply by 2^n.
	 * R0 = R1 * 8 */
	__asm__ volatile ("mov r1, #2"); /* r1=2 */
	__asm__ volatile ("mov r0, r1, lsl #3"); /* r1=2, r0=16 */
	__asm__ volatile ("mov %0, r0" : "=r" (reg)); /* r0=16 */
	printk("Register R0 << 3: %#lx\n", reg);

	return 0;
}
device_initcall(debug_mov);
