/*
 * ARM inline-assembly/Assembly: .pushsection
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
#include <linux/compiler.h>

/* Function type */
typedef int (*biscuitos_t)(void);

int biscuitos_show(void)
{
	printk("Hello BiscuitOS\n");
	return 0;
}

static void patch_hot(void)
{
	asm volatile (
			".pushsection .biscuitos,\"a\"		\n\r"
			".long	biscuitos_show			\n\r"
			".popsection");
}

/* Running .biscuitos section */
static void section_run(void)
{
	extern char __start_biscuitos[];
	extern char __end_biscuitos[]; 
	biscuitos_t *fn;

	for (fn = (biscuitos_t *)(unsigned long)__start_biscuitos; 
		fn < (biscuitos_t *)(unsigned long)__end_biscuitos; fn++) {
		(*fn)();
	}
}

static int __init mov_init(void)
{
	int idx;

	for (idx = 0; idx < 8; idx++) {
		printk("Hot plugin .biscuitos section.\n");
		patch_hot();
		section_run();
	}

	return 0;
}

/* Module exit entry */
static void __exit mov_exit(void)
{
}

module_init(mov_init);
module_exit(mov_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("ARM inline-assembly/Assembly");
