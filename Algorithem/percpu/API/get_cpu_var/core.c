/*
 * percpu
 *
 * (C) 2020.02.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/percpu.h>

/* Defind a regular Per-CPU */
DEFINE_PER_CPU(unsigned int, bs);
DECLARE_PER_CPU(unsigned int, bs);

/* Module initialize entry */
static int __init Demo_init(void)
{
	unsigned int *p = &get_cpu_var(bs);
	unsigned int cpu;

	/* set current cup val */
	*p = 0x9190;
	put_cpu_var(bs);

	for_each_possible_cpu(cpu) {
		p = &per_cpu(bs, cpu);
		printk("CPU-%d: %#x\n", cpu, *p);
	}

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
MODULE_DESCRIPTION("Per-cpu");
