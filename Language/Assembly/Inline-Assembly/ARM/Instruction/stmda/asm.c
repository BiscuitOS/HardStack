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
 * STMDA (Store Multiple) stores a non-empty subset (or possibly all) 
 * of the general-purpose registers to sequential memory locations.
 *
 * Syntax
 *   STM{<cond>}<addressing_mode> <Rn>{!}, <registers>
 */

static int debug_stmda(void)
{
	unsigned long B0[] = {0x55, 0x66, 0x77, 0x88};
	unsigned long R0[] = {0x11, 0x22, 0x33, 0x44};
	unsigned long R1;
	unsigned long R2;
	unsigned long R3;

	/*
	 * STMDA: pre-increment load
	 *
	 *  STMDA R0! {R1, R2, R3}
	 *
	 *          +--------------+
	 *          |     0x44     |
	 *          +--------------+
	 *          |     0x33     |
	 *          +--------------+        
	 *          |     0x22     |
	 *          +--------------+      
	 *          |     0x11     |       +--------------+
	 *  R0[]--->+--------------+       |   R3: 0x88   |
	 *          |     0x88    -|------>+--------------+
	 *          +--------------+       |   R2: 0x77   |
	 *          |     0x77    -|------>+--------------+
	 *          +--------------+       |   R1: 0x66   |
	 *          |     0x66    -|------>+--------------+
	 *          +--------------+
	 *          |     0x55     |
	 *  B0[]--->+--------------+
	 */
	__asm__ volatile ("stmda %3!, {%0, %1, %2}"
			: "=r" (R1), "=r" (R2), "=r" (R3)
			: "r" (R0));

	printk("STMDA: R1: %#lx R2: %#lx R3: %#lx\n", R1, R2, R3);

	return 0;
}
device_initcall(debug_stmda);
