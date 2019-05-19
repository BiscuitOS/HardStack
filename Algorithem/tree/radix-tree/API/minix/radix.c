/*
 * Radix tree.
 *
 * (C) 2019.05.09 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of radix-tree */
#include <linux/radix-tree.h>

/* node */
struct node {
	char *name;
	unsigned long id;
};

/* Radix-tree root */
static struct radix_tree_root BiscuitOS_root;

/* node */
static struct node node0 = { .name = "IDA", .id = 0x20000 };
static struct node node1 = { .name = "IDB", .id = 0x60000 };
static struct node node2 = { .name = "IDC", .id = 0x80000 };
static struct node node3 = { .name = "IDD", .id = 0x30000 };
static struct node node4 = { .name = "IDE", .id = 0x90000 };

static __init int radix_demo_init(void)
{
	struct node *np;

	/* Initialize Radix-tree root */
	INIT_RADIX_TREE(&BiscuitOS_root, GFP_ATOMIC);

	/* Insert node into Radix-tree */
	radix_tree_insert(&BiscuitOS_root, node0.id, &node0);
	radix_tree_insert(&BiscuitOS_root, node1.id, &node1);
	radix_tree_insert(&BiscuitOS_root, node2.id, &node2);
	radix_tree_insert(&BiscuitOS_root, node3.id, &node3);
	radix_tree_insert(&BiscuitOS_root, node4.id, &node4);

	/* search struct node by id */
	np = radix_tree_lookup(&BiscuitOS_root, 0x60000);
	BUG_ON(!np);
	printk("Radix: %s id %#lx\n", np->name, np->id);

	/* Delect a node from radix-tree */
	radix_tree_delete(&BiscuitOS_root, np->id);

	return 0;
}
device_initcall(radix_demo_init);
