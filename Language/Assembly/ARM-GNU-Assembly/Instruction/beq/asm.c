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
 * BEQ ---> ==  ---> equal
 *
 * Syntax
 *   BEQ <Branch>
 */

static int debug_beq(void)
{
	unsigned long R1 = 0x8;
	unsigned long R2 = 0x8;
	unsigned long RET;

	/* BEQ <branch> --> equal and jump to branck */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "beq _equ\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_equ:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx == %#lx", R1, R2);
	else
		printk("%#lx != %#lx", R1, R2);

	return 0;
}
device_initcall(debug_beq);
