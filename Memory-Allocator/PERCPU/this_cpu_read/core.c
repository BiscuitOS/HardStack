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
	int index;
};

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct bs_struct __percpu *bs;
	struct bs_struct *bp;
	int value;
	int cpu;

	/* Allocate percpu */
	bs = alloc_percpu(struct bs_struct);

	for_each_possible_cpu(cpu) {
		bp = per_cpu_ptr(bs, cpu);
		bp->index = 0x90 + cpu;
	}

	/* Read 1 */
	bp = raw_cpu_ptr(bs);
	printk("Read value %#x\n", bp->index);

	/* Read 2 */
	value = this_cpu_read(bs->index);
	printk("BV %#x\n", value);

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
