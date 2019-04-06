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
 * STMIB (Store Multiple) stores a non-empty subset (or possibly all) 
 * of the general-purpose registers to sequential memory locations.
 *
 * Syntax
 *   STM{<cond>}<addressing_mode> <Rn>{!}, <registers>
 */

static unsigned long R0[10];

static int debug_stmib(void)
{
	unsigned long R1 = 0x11;
	unsigned long R2 = 0x22;
	unsigned long R3 = 0x33;
	int i;
	
	/*
	 * STMIB: Store Register into memory, and Increment Before
	 *
	 *
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |<------ R1
	 *    R0[5]--> +-------------+
	 *             |             |<------ R2
	 *             +-------------+
	 *             |             |<------ R3
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *             +-------------+
	 *             |             |
	 *    R0[0]--> +-------------+
	 *
	 * Push register into stack.
	 */
	__asm__ volatile ("stmib %0!, {%3, %2, %1}"
			:: "r" (&R0[5]), "r" (R1), "r" (R2), "r" (R3));

	/* Emulate Stack */
	for (i = 0; i < 10; i++)
		printk("R0[%d] %#lx\n", i, R0[i]);

	return 0;
}
device_initcall(debug_stmib);
