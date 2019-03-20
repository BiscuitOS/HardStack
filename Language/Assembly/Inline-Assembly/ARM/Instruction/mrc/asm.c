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
 * MRC (Move to ARM Register from Coprocessor) causes a coprocessor to 
 * transfer a value to an ARM register or to the condition flags. If no
 * coprocessors can execute the instruction, an Undefined Instruction 
 * exception is generated.
 *
 * Syntax
 *   MRC{<cond>} <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 *   MRC2        <coproc>, <opcode_1>, <Rd>, <CRn>, <CRm>{, <opcode_2>}
 */

static int debug_mrc(void)
{
	unsigned long ID;

	/* MRC CP15 */
	__asm__ volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (ID));

	printk("ID: %#lx\n", ID);

	return 0;
}
device_initcall(debug_mrc);
