/*
 * XARRAY: xa_init_flags
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
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

/* XARRAY */
static struct xarray BiscuitOS_XA;

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct node *np;
	unsigned long index;

	xa_init_flags(&BiscuitOS_XA, XA_FLAGS_LOCK_IRQ);

	/* xa_load */
	xa_store(&BiscuitOS_XA, node0.id, &node0, GFP_KERNEL);
	xa_store(&BiscuitOS_XA, node1.id, &node1, GFP_KERNEL);
	xa_store(&BiscuitOS_XA, node2.id, &node2, GFP_KERNEL);

	/* Iterater over all xa */
	xa_for_each(&BiscuitOS_XA, index, np)
		printk("%s's ID: %#lx\n", np->name, index);

	printk("Hello BiscuitOS\n");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* xa_erase */
	xa_erase(&BiscuitOS_XA, node0.id);
	xa_erase(&BiscuitOS_XA, node1.id);
	xa_erase(&BiscuitOS_XA, node2.id);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
