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
 * MVN (Move Not) generates the logical ones complement of a value. The
 * value can be either an immediate value or a value from a register, 
 * and can be shifted before the MVN operation. MVN can optionally update
 * the condition code flags, based on the result.
 *
 * Syntax
 *   MVN{<cond>} {S} <Rd>, <shifter_operand>
 */

int debug_mvn(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 0xb;

	/* MVN R0, R1---> R0 = !R1 */
	__asm__ volatile ("mvn %0, %1" :: 
		"r" (a0), "r" (a1));

	printk("!%#lx = %#lx\n", a1, a0);

	return 0;
}
device_initcall(debug_mvn);
