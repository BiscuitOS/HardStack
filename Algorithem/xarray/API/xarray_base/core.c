/*
 * xarray
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

#include <linux/xarray.h>

/* node */
struct node {
	char *name;
	unsigned long id;
};

/* static declar and initialize node */
static struct node node0 = { .name = "BiscuitOS_A", .id = 0x45888 };
static struct node node1 = { .name = "BiscuitOS_B", .id = 0x89889 };
static struct node node2 = { .name = "BiscuitOS_C", .id = 0x62668 };

/* Xarry root */
static DEFINE_XARRAY(BiscuitOS_xa);

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct node *np;
	unsigned long index;

	/* Insert node into Xarray */
	xa_store(&BiscuitOS_xa, node0.id, &node0, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node1.id, &node1, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node2.id, &node2, GFP_KERNEL);

	/* Iterater node on Xarray */
	xa_for_each(&BiscuitOS_xa, index, np)
		printk("BiscuitOS_xa[%#lx]%s's ID %#lx\n", index,
							np->name, np->id);

	/* Search node by id */
	np = xa_load(&BiscuitOS_xa, node0.id);
	printk("Find %s\n", np->name);

	/* Remove node from Xarray */
	xa_erase(&BiscuitOS_xa, node0.id);
	xa_erase(&BiscuitOS_xa, node1.id);
	xa_erase(&BiscuitOS_xa, node2.id);

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
MODULE_DESCRIPTION("Xarray Machanism");
