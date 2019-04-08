/*
 * ARM Assembly
 *
 * (C) 2019.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * CLZ
 *
 *   CLZ (Count Leading Zeros) returns the number of binary zero bits before
 *   the first binary one bit in a value. CLZ does not update the condition 
 *   code flags.
 *
 *   Syntax
 *     CLZ{<cond>} <Rd>, <Rm>
 */

static int debug_CLZ(void)
{
	unsigned long R0 = 0x48;
	unsigned long R1;

	/* CLZ: Count zero for highest 1 to MSB, e.g.
	 *
	 * 1) R0: 0x2   R1:0
	 *        CLZ R1, R0
	 *    Binary of R0: 0000 0000 0000 0000 0000 0000 0000 0010
	 *                  | <-------------------------------> |
	 *                            Zero number: 0x1e
	 *    So, R1 is '0x1e'.
	 * 2) R0: 0x48  R1:0
	 *        CLZ R1, R0
	 *    Binary of R0: 0000 0000 0000 0000 0000 0000 0100 1000
	 *                  | <-------------------------> |
	 *                          Zero number: 0x19
	 *    So, R1 is '0x19'
	 */
	__asm__ volatile ("clz %0, %1" : "=r" (R1) : "r" (R0));
	printk("The zero number of for %#lx's left of highest 1: %#lx\n",
			R0, R1);

	return 0;
}
device_initcall(debug_CLZ);
