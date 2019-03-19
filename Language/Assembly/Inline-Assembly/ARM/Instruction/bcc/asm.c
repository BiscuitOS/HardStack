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
 * BCC ---> !C  ---> Non-Carry
 *
 * Syntax
 *   BCC <Branch>
 */

static int debug_bcc(void)
{
	unsigned long R1 = 0x2;
	unsigned long R2 = 0x5;
	unsigned long RET;

	/* BCC <branch> --> No Carry and jump to branck */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "bcc _nc\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_nc:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx - %#lx no Carry!", R1, R2);
	else
		printk("%#lx - %#lx Carry!", R1, R2);

	return 0;
}
device_initcall(debug_bcc);
