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
 * ADD adds two values. The first value comes from a register. The second 
 * value can be either an immediate value or a value from a register, and
 * can be shifted before the addition. ADD can optionally update the 
 * condition code flags, based on the result.
 *
 * Syntax
 *  ADD {<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 *
 * Usage
 *
 * - Use ADD to add two values together.
 * - To increment a register value in Rx use:
 *        ADD Rx, Rx, #1
 * - You can perform constant multiplication of Rx by 2 n +1 into Rd with:
 *        ADD Rd, Rx, Rx, LSL #n
 * - To form a PC-relative address use:
 *        ADD Rd, PC, #offset
 *   where the offset must be the difference between the required address 
 *   and the address held in the PC, where the PC is the address of the 
 *   ADD instruction itself plus 8 bytes.
 */

static int debug_add(void)
{
	unsigned long a0 = 0;
	unsigned long a1 = 2;
	unsigned long a2 = 1;

	/* ADD R0, R1, R2 ---> R0 = R1 + R2 */
	__asm__ volatile ("add %0, %1, %2" :: 
		"r" (a0), "r" (a1), "r" (a2));

	printk("%#lx + %#lx = %#lx\n", a1, a2, a0);

	a0 = 0;
	/* ADD R0, R1, R2, LSL #n --> R0 = R1 + (R2 << n) */
	__asm__ volatile ("add %0, %1, %2, lsl #3" ::
		"r" (a0), "r" (a1), "r" (a2));

	printk("%#lx + %#lx << 3 = %#lx\n", a1, a2, a0);

	return 0;
}
device_initcall(debug_add);
