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
 * TST (Test) compares a register value with another arithmetic value. 
 * The condition flags are updated, based on the result of logically 
 * ANDing the two values, so that subsequent instructions can be 
 * conditionally executed.
 *
 * Syntax
 *   TST{<cond>} <Rn>, <shifter_operand>
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

static int debug_tst(void)
{
	unsigned long R1 = 4;
	unsigned long R2 = 2;
	unsigned long RET;

	/* TST R1, R2 --> Test R1 AND R2 */
	__asm__ volatile ("tst %1, %2\n\r"
			  "bne _br\n\r"
			  "mov %0, #0\n\r"
			  "b out\n\r"
		"_br:\n\r"
			  "mov %0, #1\n\r"
		"out:\n\r"
			  "nop"
			: "=r" (RET)
			: "r" (R1), "r" (R2));

	if (!RET)
		printk("%#lx bit %#lx is set", R1, R2);
	else
		printk("%#lx bit %#lx is clear", R1, R2);

	return 0;
}
device_initcall(debug_tst);
