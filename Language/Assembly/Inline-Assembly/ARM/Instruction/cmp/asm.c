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
 * CMP (Compare) compares two values. The first value comes from a register.
 * The second value can be either an immediate value or a value from a 
 * register, and can be shifted before the comparison. CMP updates the 
 * condition flags, based on the result of subtracting the second value
 * from the first.
 *
 * Syntax
 *   CMP{<cond>} <Rn>, <shifter_operand>
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

static int debug_cmp(void)
{
	unsigned long R1 = 0x2;
	unsigned long R2 = 0x1;
	unsigned long RET;

	/* CMP R1, R2 ---> Compare R1, R2 ---> (R1-R2) ---> CPRS */
	__asm__ volatile ("cmp %1, %2\n\r"
			  "bhi _big\n\r"
			  "bls _less\n\r"
		"_big:\n\r"
			  "mov %0, #1\n\r"
			  "b out\n\r"
		"_less:\n\r"
			  "mov %0, #0\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (RET)
		printk("%#lx > %#lx", R1, R2);
	else
		printk("%#lx < %#lx", R1, R2);

	return 0;
}
device_initcall(debug_cmp);
