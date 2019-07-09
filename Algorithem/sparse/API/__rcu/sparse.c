/*
 * Sparse.
 *
 * (C) 2019.07.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* sparse macro */
#include <linux/types.h>

static __init int sparse_demo_init(void)
{
	unsigned int __rcu *src;
	unsigned int dst = 0x10;
	unsigned int __rcu data = 0x20;

	/* RCU points to kernel space */
	src = &dst;

	/* RCU points to RCU space */
	src = &data;

	return 0;
}
device_initcall(sparse_demo_init);
