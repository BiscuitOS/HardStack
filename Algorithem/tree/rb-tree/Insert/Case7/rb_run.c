/*
 * RB-Tree Manual.
 *
 * (C) 2019.05.14 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* rbtree */
#include <rbtree.h>

struct node {
	struct rb_node node;
	unsigned long runtime;
};

/* n points to a rb_node */
#define node_entry(n) container_of(n, struct node, node)

static struct node node0 = { .runtime = 0x20 };
static struct node node1 = { .runtime = 0x30 };
static struct node node2 = { .runtime = 0x25 };

/* rbroot */
static struct rb_root BiscuitOS_rb = RB_ROOT;

/* Insert private node into rbtree */
static int rbtree_insert(struct rb_root *root, struct node *node)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct node *this = node_entry(*new);
		int result;

		/* Compare runtime */
		result = this->runtime - node->runtime;

		/* setup parent */
		parent = *new;

		/*
		 *        (this)
		 *         /  \
		 *        /    \
		 *  (little)   (big)
		 *
		 */
		if (result < 0)
			new = &((*new)->rb_right);
		else if (result > 0)
			new = &((*new)->rb_left);
		else
			return 0;
	}

	/* Add new node and rebalance tree */
	rb_link_node(&node->node, parent, new);
	rb_insert_color(&node->node, root);
}

static int count = 20;
/* Middle-order iterate over RB tree */
static void Middorder_IterateOver(struct rb_node *node)
{
	if (!node) {
		return;
	} else {
		Middorder_IterateOver(node->rb_left);
		printf("%#lx ", node_entry(node)->runtime);
		Middorder_IterateOver(node->rb_right);
	}
}

int main()
{
	struct node *np;

	/* Insert rb_node */
	rbtree_insert(&BiscuitOS_rb, &node0);
	rbtree_insert(&BiscuitOS_rb, &node1);
	rbtree_insert(&BiscuitOS_rb, &node2);
	Middorder_IterateOver(BiscuitOS_rb.rb_node);
	printf("\n");

	return 0;
}
