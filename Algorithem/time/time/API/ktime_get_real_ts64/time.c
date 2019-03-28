/*
 * Kernel real time
 *
 * (C) 2019.03.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/timex.h>
#include <linux/timekeeper_internal.h>
#include <generated/timeconst.h>

static int time_demo(void)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	printk("TIME[%lld:%ld]\n", ts.tv_sec, ts.tv_nsec);

	return 0;
}
device_initcall(time_demo);
