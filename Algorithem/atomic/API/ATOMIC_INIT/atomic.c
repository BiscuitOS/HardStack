/*
 * atomic
 *
 * (C) 2019.05.05 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* atomic_* */
#include <asm/atomic.h>

static atomic_t BiscuitOS_counter = ATOMIC_INIT(8);

static __init int atomic_demo_init(void)
{
	printk("ATOMIC init: %d\n", atomic_read(&BiscuitOS_counter));

	return 0;
}
device_initcall(atomic_demo_init);
