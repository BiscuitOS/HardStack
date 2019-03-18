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

#include <linux/mm.h>

/*
 * STR (Store Register) stores a word from a register to memory.
 *
 * Syntax
 *   STR{<cond>} <Rd>, <addressing_mode>
 *
 * Usage
 *
 * - Combined with a suitable addressing mode, STR stores 32-bit data 
 *   from a general-purpose register into memory. Using the PC as the
 *   base register allows PC-relative addressing, which facilitates 
 *   position-independent code.
 */

static int debug_str(void)
{
	pgd_t *pgd = swapper_pg_dir;
	unsigned long R0 = 0x55AA;
	unsigned long R1 = (unsigned long)pgd;
	unsigned long R2;
	
	/* 
	 * Store vlaue into memory 
	 *   STR Rn, [R1] --> Store Rn value into R1 memory.
	 */
	__asm__ volatile ("str %0, [%1]" :: "r" (R0), "r" (R1));
	printk("Store %#lx into %#lx\n", (unsigned long)pgd_val(pgd[0]), R1);

	/*
	 * Store value into memory with index
	 *   STR Rn, [R1, R2] --> Store Rn value into (R1+R2) memory.
	 */
	R2 = 0x8;
	__asm__ volatile ("str %0, [%1, %2]" :: "r" (R0), "r" (R1), "r" (R2));
	printk("Store %#lx into (%#lx+%#lx)\n", 
			(unsigned long)pgd_val(pgd[1]), R1, R2);
	

	/*
	 * Store value into memory with index, and update dest-address
	 *   LDR Rn, [R1, R2]! --> Store Rn value into (R1+R2) memory, and
	 *                         update R1 address as (R1+R2).
	 */
	R2 = 0x16;
	__asm__ volatile ("str %0, [%1, %2]!" :: "r" (R0), "r" (R1), "r" (R2));
	printk("Store %#lx into (%#lx+%#lx) --> R1: %#lx\n",
			(unsigned long)pgd_val(pgd[3]), 
			(unsigned long)pgd, R2, R1);

	/*
	 * Store value into memory and update dest-address.
	 *   LDR Rn, [R1], R2 --> Store Rn value into R1, and update R1
	 *                        address as (R1 + R2)
	 */
	R2 = 0x1;
	__asm__ volatile ("str %0, [%1], %2" :: "r" (R0), "r" (R1), "r" (R1));
	printk("Store %#lx into %#lx --> R1: %#lx\n", R0, 
			(unsigned long)pgd, R1);

	return 0;
}
device_initcall(debug_str);
