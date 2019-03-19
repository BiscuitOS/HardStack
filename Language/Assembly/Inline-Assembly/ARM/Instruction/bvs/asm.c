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
 * BVS ---> O   ---> Overflow with signed-
 *
 * Syntax
 *   BVS <Branch>
 */

static int debug_bvs(void)
{
	long R1 = -324234;
	long R2 = 45;
	unsigned long RET;

	/* BVS <branch> --> Overflow and jump to branck */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "bvs _vs\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_vs:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx - %#lx Overflow", R1, R2);
	else
		printk("%#lx - %#lx not Overflow", R1, R2);

	return 0;
}
device_initcall(debug_bvs);
