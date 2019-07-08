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
	int __kernel *src;
	int __user dst = 0x10;

	/* points to user space */
	src = &dst;

	printk("SRC: %#x\n", *src);

	return 0;
}
device_initcall(sparse_demo_init);
