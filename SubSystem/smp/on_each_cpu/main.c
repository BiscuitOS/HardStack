/*
 * Call a function on all processors
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

void BiscuitOS_smp(void *info)
{
	int cpu = raw_smp_processor_id();

	printk("Current CPU%d Default CPU%d\n", cpu, (int)info);
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int cpu = raw_smp_processor_id();

	/* Call function on all processor */
	on_each_cpu(BiscuitOS_smp, (void *)cpu, 0);

	printk("Hello modules on BiscuitOS\n");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
