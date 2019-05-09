/*
 * Xarray.
 *
 * (C) 2019.05.09 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of xarray */
#include <linux/xarray.h>

/* node */
struct node {
	char *name;
	unsigned long idx;
};

static struct node node0 = { .name = "IDA", .idx = 0x2344 };

/* Xarray root */
DEFINE_XARRAY(BiscuitOS_root);

static __init int xarray_demo_init(void)
{
	struct xa_state BiscuitOS_xas = 
			__XA_STATE(&BiscuitOS_root, node0.idx, 0, 0);

	printk("xas.index: %#lx\n", BiscuitOS_xas.xa_index);
	printk("xas.xa_node: %#lx\n", (unsigned long)BiscuitOS_xas.xa_node);

	printk("xarray done......\n");

	return 0;
}
device_initcall(xarray_demo_init);
