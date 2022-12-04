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

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct bs_struct __percpu *bs;
	struct bs_struct *bp;
	unsigned int cpu;

	/* Allocate percpu */
	bs = alloc_percpu(struct bs_struct);

	/* setup private */
	bp = get_cpu_ptr(bs);
	bp->info = 0x91;
	put_cpu_ptr(bs);

	/* output */
	for_each_possible_cpu(cpu) {
		bp = per_cpu_ptr(bs, cpu);
		printk("CPU-%d INFO %#x\n", cpu, bp->info);
	}

	/* free */
	free_percpu(bs);
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
