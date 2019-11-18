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
#include <linux/time.h>

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct timespec64 start;
	struct timespec64 end;
	struct timespec64 duration;

	/* Timing start */
	ktime_get_ts64(&start);

	printk("Timing\n");
	
	/* Timing end */
	ktime_get_ts64(&end);

	/* Calculate Timing */
	duration = timespec64_sub(end, start);

	/* output */
	printk("Second:      %lld\n", duration.tv_sec);
	printk("Millisecond: %ld\n", duration.tv_nsec / NSEC_PER_USEC);

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
