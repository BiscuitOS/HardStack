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

struct bs_struct {
	int cpu;
	int info;
	char *name;
};

/* Defind a regular Per-CPU */
DEFINE_PER_CPU(struct bs_struct, bs);
DECLARE_PER_CPU(struct bs_struct, bs);

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct bs_struct *bp;
	unsigned int cpu;

	/* setup all */
	for_each_possible_cpu(cpu) {
		bp = per_cpu_ptr(&bs, cpu);
		bp->cpu = cpu;
		bp->name = kasprintf(GFP_KERNEL, "%s-%d", "CPU", cpu);
	}

	/* setup private */
	bp = per_cpu_ptr(&bs, 1);
	bp->info = 0x91;

	/* output */
	for_each_possible_cpu(cpu) {
		bp = per_cpu_ptr(&bs, cpu);
		printk("%s INFO %#x\n", bp->name, bp->info);
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
