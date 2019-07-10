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
 * CPS (Change Processor State) changes one or more of the mode, A, I, 
 *     and F bits of the CPSR, without changing the other CPSR bits.
 * Syntax
 *     CPS<effect> <iflags> {, #<mode>}
 *
 * <effect>
 *     Specifies what effect is wanted on the interrupt disable bits A, 
 *     I, and F in the CPSR. This is one of:
 *        IE    Interrupt Enable, encoded by imod == 0b10. This sets the 
 *              specified bits to 0.
 *        ID    Interrupt Disable, encoded by imod == 0b11. This sets 
 *              the specified bits to 1.
 *     If <effect> is specified, the bits to be affected are specified 
 *     by <iflags> . These are encoded in the A, I, and F bits of the 
 *     instruction. The mode can optionally be changed by specifying
 *     a mode number as <mode>.
 *     If <effect> is not specified, then:
 *     •   <iflags> is not specified and the A, I, and F mask settings are
 *         not changed
 *     •   the A, I, and F bits of the instruction are zero
 *     •   imod = 0b00
 *     •   mmod = 0b1
 *     •   <mode> specifies the new mode number.
 * <iflags>
 *     Is a sequence of one or more of the following, specifying which
 *     interrupt disable flags are affected:
 *        a     Sets the A bit in the instruction, causing the specified
 *              effect on the CPSR A (imprecise data abort) bit.
 *        i     Sets the I bit in the instruction, causing the specified
 *              effect on the CPSR I (IRQ interrupt) bit.
 *        f     Sets the F bit in the instruction, causing the specified
 *              effect on the CPSR F (FIQ interrupt) bit.
 * <mode>
 *     Specifies the number of the mode to change to. If it is present, 
 *     then mmod == 1 and the mode number is encoded in the mode field of
 *     the instruction. If it is omitted, then mmod == 0 and the mode 
 *     field of the instruction is zero.
 */

static int debug_asm(void)
{
	/* disable interrupt */
	__asm__ volatile ("cpsid i	@ arch_local_irq_disable"
			  ::: "memory", "cc");

	/* enable interrupt */
	__asm__ volatile ("cpsie i	@ arch_local_irq_enable"
			  ::: "memory", "cc");

	return 0;
}
device_initcall(debug_asm);
