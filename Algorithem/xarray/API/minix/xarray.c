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
	unsigned long id;
};

/* static node declear and initialize */
static struct node node0 = { .name = "XA", .id = 0x455 };
static struct node node1 = { .name = "XB", .id = 0x875 };
static struct node node2 = { .name = "XC", .id = 0x137 };

/* Xarray root */
static struct xarray BiscuitOS_root;
static int counter = 0;

/* insert op */
static int xarray_store(struct xarray *root, unsigned long index, void *entry)
{
	int err;

	xa_lock_bh(root);
	err = xa_err(__xa_store(root, index, entry, GFP_KERNEL));
	if (!err)
		counter++;
	xa_unlock_bh(root);
	return err;
}

/* erase op */
static void xarray_erase(struct xarray *root, unsigned long index)
{
	xa_lock(root);
	__xa_erase(root, index);
	counter--;
	xa_unlock(root);
}

static __init int xarray_demo_init(void)
{
	/* init xarray */
	xa_init(&BiscuitOS_root);

	/* xa_load */
	xarray_store(&BiscuitOS_root, node0.id, &node0);
	xarray_store(&BiscuitOS_root, node1.id, &node1);
	xarray_store(&BiscuitOS_root, node2.id, &node2);

	/* xa_erase */
	xarray_erase(&BiscuitOS_root, node0.id);
	xarray_erase(&BiscuitOS_root, node1.id);
	xarray_erase(&BiscuitOS_root, node2.id);

	printk("xarray done......\n");

	return 0;
}
device_initcall(xarray_demo_init);
