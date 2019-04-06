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
 * B (Branch) cause a branch to a target address, and provide both 
 * conditional and unconditional changes to program flow.
 *
 * Syntax
 *   B{L}{<cond>} <target_address>
 *
 * BEQ ---> ==  ---> equal
 * BNE ---> !=  ---> Unequal
 * BPL ---> > 0 ---> N0nNegative number
 * BMI ---> < 0 ---> Negative number
 * BCC ---> !C  ---> Non-Carry
 * BCS ---> C   ---> Carry
 * BLO ---> < 0 ---> Less then with unsigned-
 * BHS ---> >=0 ---> Big then and equal with unsigned-
 * BHI ---> >   ---> Big then with unsigned-
 * BLS ---> <=  ---> Less than and equal with unsigned-
 * BVC ---> !O  ---> No overflow with signed-
 * BVS ---> O   ---> Overflow with signed-
 * BGT ---> >   ---> Big then with signed-
 * BGE ---> >=  ---> Big and equal with signed-
 * BLT ---> <   ---> Less then with signed-
 * BLE ---> <=  ---> Less and equal with signed-
 */

static int debug_b(void)
{
	unsigned long RET;

	__asm__ volatile ("b _br\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_br:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET));

	printk("BR: %#lx\n", RET);

	return 0;
}
device_initcall(debug_b);
