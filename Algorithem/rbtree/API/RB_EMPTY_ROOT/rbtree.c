/*
 * rbtree
 *
 * (C) 2019.05.08 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of rbtree */
#include <linux/rbtree.h>

/* rbtree */
struct node {
	struct rb_node node;
	unsigned long runtime;
};

static struct node node0 = { .runtime = 0x20 };
static struct node node1 = { .runtime = 0x40 };
static struct node node2 = { .runtime = 0x60 };
static struct node node3 = { .runtime = 0x10 };
static struct node node4 = { .runtime = 0x01 };
static struct node node5 = { .runtime = 0x53 };
static struct node node6 = { .runtime = 0x24 };
static struct node node7 = { .runtime = 0x89 };
static struct node node8 = { .runtime = 0x56 };
static struct node node9 = { .runtime = 0x43 };

/* root for rbtree */
struct rb_root BiscuitOS_rb = RB_ROOT;

/* Insert private node into rbtree */
static int rbtree_insert(struct rb_root *root, struct node *node)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct node *this = container_of(*new, struct node, node);
		int result;

		/* Compare runtime */
		result = this->runtime - node->runtime;

		/* setup parent */
		parent = *new;
	
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return 0;
	}

	/* Add new node and rebalance tree */
	rb_link_node(&node->node, parent, new);
	rb_insert_color(&node->node, root);

	return 1;
}

/* Search private node on rbtree */
struct node *rbtree_search(struct rb_root *root, unsigned long runtime)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct node *this = container_of(node, struct node, node);
		int result;

		result = this->runtime - runtime;

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return this;
	}
	return NULL;
}

static __init int rbtree_demo_init(void)
{
	struct rb_node *np;
	struct node *this;

	if (RB_EMPTY_ROOT(&BiscuitOS_rb))
		printk("RB tree is empty.\n");

	/* Insert rb_node */
	rbtree_insert(&BiscuitOS_rb, &node0);
	rbtree_insert(&BiscuitOS_rb, &node1);
	rbtree_insert(&BiscuitOS_rb, &node2);
	rbtree_insert(&BiscuitOS_rb, &node3);
	rbtree_insert(&BiscuitOS_rb, &node4);
	rbtree_insert(&BiscuitOS_rb, &node5);
	rbtree_insert(&BiscuitOS_rb, &node6);
	rbtree_insert(&BiscuitOS_rb, &node7);
	rbtree_insert(&BiscuitOS_rb, &node8);
	rbtree_insert(&BiscuitOS_rb, &node9);

	if (!RB_EMPTY_ROOT(&BiscuitOS_rb))
		printk("RB tree isn't empty.\n");

	/* Traverser all node on rbtree */
	for (np = rb_first(&BiscuitOS_rb); np; np = rb_next(np))
		printk("RB: %#lx\n", rb_entry(np, struct node, node)->runtime);

	/* Search node by runtime */
	this = rbtree_search(&BiscuitOS_rb, 0x53);
	if (this) {
		struct rb_node *parent;

		/* Obtain rb_node's parent */
		parent = rb_parent(&this->node);
		if (parent)
			printk("%#lx's parent is %#lx\n", this->runtime, 
				rb_entry(parent, struct node, node)->runtime);
		else
			printk("illegae child\n");
		
	} else
		printk("Invalid data on rbtree\n");

	/* delete rb_node */
	rb_erase(&node0.node, &BiscuitOS_rb);
	rb_erase(&node3.node, &BiscuitOS_rb);
	rb_erase(&node4.node, &BiscuitOS_rb);
	rb_erase(&node6.node, &BiscuitOS_rb);
	printk("Remove: %#lx %#lx %#lx %#lx\n", node0.runtime, node3.runtime,
				node4.runtime, node6.runtime);

	printk("Traverser all node again\n");
	/* Traverser all node again */
	for (np = rb_first(&BiscuitOS_rb); np; np = rb_next(np))
		printk("RB: %#lx\n", rb_entry(np, struct node, node)->runtime);

	return 0;
}
device_initcall(rbtree_demo_init);
