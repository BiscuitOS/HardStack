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

	/* CPU0 */
	cpu = 0;
	bp = &per_cpu(bs, cpu);
	bp->info = 0x90;
	/* CPU1 */
	cpu = 1;
	bp = &per_cpu(bs, cpu);
	bp->info = 0x91;

	for_each_possible_cpu(cpu) {
		bp = &per_cpu(bs, cpu);
		printk("CPU-%d: %#x\n", cpu, bp->info);
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
