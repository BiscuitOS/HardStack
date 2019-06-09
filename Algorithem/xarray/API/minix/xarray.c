/*
 * Xarray.
 *
 * (C) 2019.06.06 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>

/* header of xarray */
#include <linux/xarray.h>

/* node */
struct node {
	char *name;
	unsigned long id;
};

/* static node declear and initialize */
static struct node node0 = { .name = "XA", .id = 0x455 };
static struct node node1 = { .name = "XB", .id = 0x875 };
static struct node node2 = { .name = "XC", .id = 0x137 };

/* Xarray root */
static DEFINE_XARRAY(BiscuitOS_xa);

static __init int xarray_demo_init(void)
{
	struct node *np;
	unsigned long index;

	/* xa_load */
	xa_store(&BiscuitOS_xa, node0.id, &node0, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node1.id, &node1, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node2.id, &node2, GFP_KERNEL);

	/* Iterater over all xa */
	xa_for_each(&BiscuitOS_xa, index, np)
		printk("%s's ID: %#lx\n", np->name, index);

	/* xa_erase */
	xa_erase(&BiscuitOS_xa, node0.id);
	xa_erase(&BiscuitOS_xa, node1.id);
	xa_erase(&BiscuitOS_xa, node2.id);

	return 0;
}
device_initcall(xarray_demo_init);
