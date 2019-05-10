/*
 * RCU lock.
 *
 * (C) 2019.05.09 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* rcu lock */
#include <linux/rcupdate.h>

static __init int rcu_demo_init(void)
{
	/* acquire rcu lock */
	rcu_read_lock();

	__asm__ volatile ("nop");
	
	/* release rcu lock */
	rcu_read_unlock();

	printk("RCU lock done......\n");

	return 0;
}
device_initcall(rcu_demo_init);
