/*
 * ARM inline-assembly/Assembly: BIC
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/*
 * BIC (Bit Clear) performs a bitwise AND of one value with the complement 
 * of a second value. The first value comes from a register. The second 
 * value can be either an immediate value or a value from a register, and 
 * can be shifted before the BIC operation. BIC can optionally update the 
 *condition code flags, based on the result.
 *
 * Syntax
 *   BIC{<cond>} {S} <Rd>, <Rn>, <shifter_operand>
 */

/* Module initialize entry */
static int __init bic_init(void)
{

	unsigned long a0 = 0;
	unsigned long a1 = 0xFFFF;
	unsigned long a2 = 0xb;

	/* BIC R0, R1, R2 ---> R0 = R1 Clear bit on R2  */
	__asm__ volatile ("bic r0, %1, %2\n\r" 
			  "mov %0, r0"
			: "=r" (a0)
        		: "r" (a1), "r" (a2));

	printk("%#lx Clear %#lx = %#lx\n", a1, a2, a0);

	return 0;
}

/* Module exit entry */
static void __exit bic_exit(void)
{
}

module_init(bic_init);
module_exit(bic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("ARM inline-assembly/Assembly");
