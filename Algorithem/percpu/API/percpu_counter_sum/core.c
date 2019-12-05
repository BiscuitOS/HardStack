/*
 * BiscuitOS
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
#include <linux/percpu_counter.h>

static struct percpu_counter BiscuitOS_nr __cacheline_aligned_in_smp;

/* Module initialize entry */
static int __init Demo_init(void)
{
	/* percpu init */
	percpu_counter_init(&BiscuitOS_nr, 20, GFP_KERNEL);
	printk("Initialize: %lld\n", percpu_counter_sum(&BiscuitOS_nr));

	/* dec */
	percpu_counter_dec(&BiscuitOS_nr);
	printk("Dec: %lld\n", percpu_counter_sum(&BiscuitOS_nr));

	/* inc */
	percpu_counter_inc(&BiscuitOS_nr);
	printk("Inc: %lld\n", percpu_counter_sum(&BiscuitOS_nr));

	/* sub */
	percpu_counter_sub(&BiscuitOS_nr, 6);
	printk("Sub: %lld\n", percpu_counter_sum(&BiscuitOS_nr));

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	/* Destroy */
	percpu_counter_destroy(&BiscuitOS_nr);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS core");
