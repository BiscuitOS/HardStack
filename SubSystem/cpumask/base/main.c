/*
 * CPU mask mechanism
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>

/* CPUmask */
#include <linux/cpumask.h>

/* declear cpumask */
static cpumask_var_t BiscuitOS_cpumask;

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int cpu = raw_smp_processor_id();

	printk("BiscuitOS current cpu %d\n", cpu);
	/* alloc cpumask */
	zalloc_cpumask_var(&BiscuitOS_cpumask, GFP_KERNEL);

	/* test and set */
	if (!cpumask_test_cpu(cpu, BiscuitOS_cpumask)) {
		printk("CPUMASK set cpu %d\n", cpu);
		cpumask_set_cpu(cpu, BiscuitOS_cpumask);
	}

	/* test and clear */
	if (cpumask_test_cpu(cpu, BiscuitOS_cpumask)) {
		printk("CPUMASK clear cpu %d\n", cpu);
		cpumask_clear_cpu(cpu, BiscuitOS_cpumask);
	}

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* free cpumask */
	free_cpumask_var(BiscuitOS_cpumask);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
