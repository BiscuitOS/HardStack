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
 * BMI ---> < 0 ---> Negative number
 *
 * Syntax
 *   BMI <Branch>
 */

static int debug_bmi(void)
{
	unsigned long R1 = 0x3;
	unsigned long R2 = 0x5;
	unsigned long RET;

	/* BMI <branch> --> Negative number and jump to branck */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "bmi _ng\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_ng:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx - %#lx < 0", R1, R2);
	else
		printk("%#lx - %#lx >= 0", R1, R2);

	return 0;
}
device_initcall(debug_bmi);
