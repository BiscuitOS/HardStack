/*
 * Device Driver
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
#include <linux/slab.h>

/* Module initialize entry */
static int __init Demo_init(void)
{
	unsigned long *addr, *p;
	unsigned long output, offset;

	/* aligned pointer address */
	addr = kzalloc(sizeof(unsigned long), GFP_KERNEL);
	*addr = 0x12345678;
	/* unalignd pointer address */
	p = (unsigned long *)((unsigned long)addr + 1);
	/* p + 0 --> 0x12345678
	 * p + 1 --> 0x123456
	 * p + 2 --> 0x1234
	 * p + 3 --> 0x12
	 */

	asm volatile (
	"1:	ldr	%0, [%2]				\n"
	"2:							\n"
	"	.pushsection .text.fixup,\"ax\"			\n"
	"	.align	2					\n"
	"3:	and	%1, %2, #0x3				\n"
	"	bic	%2, %2, #0x3				\n"
	"	ldr	%0, [%2]				\n"
	"	lsl	%1, %1, #0x3				\n"
	"	lsr	%0, %0, %1				\n"
	"	b	2b					\n"
	"	.popsection					\n"
	"	.pushsection __ex_table,\"a\"			\n"
	"	.align	3					\n"
	"	.long	1b, 3b					\n"
	"	.popsection"
	: "=&r" (output), "=&r" (offset)
	: "r" (p), "Qo" (*addr));

	printk("Default-addr: %#lx -- output %#lx offset %#lx\n", 
				(unsigned long)addr, output, offset);
	kfree(addr);

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Device driver");
