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
 * MCR (Move to Coprocessor from ARM Register) passes the value of 
 * register <Rd> to the coprocessor whose number is cp_num. If no 
 * coprocessors indicate that they can execute the instruction, an 
 * Undefined Instruction exception is generated.
 *
 * Syntax
 *   MCR{<cond>} <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 *   MCR2        <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 */

static int debug_mcr(void)
{
	unsigned long CSS;

	/* Read CSSELR into Rt */
	__asm__ volatile ("mrc p15,2,%0,c0,c0,0" : "=r" (CSS));
	printk("CSSELR: %#lx\n", CSS);

	/* Write Rt into CSSELR */
	__asm__ volatile ("mcr p15,2,%0,c0,c0,0" :: "r" (CSS));

	return 0;
}
device_initcall(debug_mcr);
