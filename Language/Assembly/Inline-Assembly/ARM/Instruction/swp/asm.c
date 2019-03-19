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
 * SWP (Swap) swaps a word between registers and memory. SWP loads a word
 * from the memory address given by the value of register <Rn> . The value
 * of register <Rm> is then stored to the memory address given by the 
 * value of <Rn> , and the original loaded value is written to register 
 * <Rd> . If the same register is specified for <Rd> and <Rm> , this 
 * instruction swaps the value of the register and the value at the memory
 * address.
 *
 * Syntax
 *   SWP{<cond>} <Rd>, <Rm>, [<Rn>]
 */

static unsigned long R2[10] = {0x99};

static int debug_swp(void)
{
	unsigned long R1 = 0x88;

	/* SWP R1, R1, [R2] ----> Exchange R2 and R1 value. */
	__asm__ volatile ("mov r1, %1\n\r"
			  "swp r1, r1, [%2]\n\r"
			  "mov r1, %0"
			: "=r" (R1) : "r" (R1), "r" (R2));
	printk("R1: %#lx R2[0]: %#lx\n", R1, R2[0]);

	/* SAP R1, R2, [R0] ---> Load R0 value into R1 */
	__asm__ volatile ("swp r1, r2, [%1]\n\r"
			  "mov r1, %0"
			: "=r" (R1) : "r" (R2));
	printk("R1: %#lx\n", R1);

	return 0;
}
device_initcall(debug_swp);
