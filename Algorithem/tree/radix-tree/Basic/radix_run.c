/*
 * Radix-Tree Manual.
 *
 * (C) 2019.05.27 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* radix-tree */
#include <radix.h>

/*
 * Radix-tree                                             RADIX_TREE_MAP: 6
 *                                  (root)
 *                                    |
 *                          o---------o---------o
 *                          |                   |
 *                        (0x0)               (0x2)
 *                          |                   |
 *                  o-------o------o            o---------o
 *                  |              |                      |
 *                (0x0)          (0x2)                  (0x2)
 *                  |              |                      |
 *         o--------o------o       |             o--------o--------o
 *         |               |       |             |        |        |
 *       (0x0)           (0x1)   (0x0)         (0x0)    (0x1)    (0x3)
 *         A               B       C             D        E        F
 *
 * A: 0x00000000
 * B: 0x00000001
 * C: 0x00000080
 * D: 0x00080080
 * E: 0x00080081
 * F: 0x00080083
 *
 */      

/* node */
struct node {
	char *name;
	unsigned long id;
};

/* Radix-tree root */
static struct radix_tree_root BiscuitOS_root;

/* Range: [0x00000000, 0x00000040] */
static struct node node0 = { .name = "IDA", .id = 0x21 };
/* Range: [0x00000040, 0x00001000] */
static struct node node1 = { .name = "IDB", .id = 0x876 };
/* Range: [0x00001000, 0x00040000] */
static struct node node2 = { .name = "IDC", .id = 0x321FE };
/* Range: [0x00040000, 0x01000000] */
static struct node node3 = { .name = "IDD", .id = 0x987654 };
/* Range: [0x01000000, 0x40000000] */
static struct node node4 = { .name = "IDE", .id = 0x321FEDCA };

int main()
{
	struct node *np;
	struct radix_tree_iter iter;
	void **slot;

	/* Initialize Radix-tree root */
	INIT_RADIX_TREE(&BiscuitOS_root, GFP_ATOMIC);

	/* Insert node into Radix-tree */
	radix_tree_insert(&BiscuitOS_root, node0.id, &node0);
//	radix_tree_insert(&BiscuitOS_root, node1.id, &node1);
//	radix_tree_insert(&BiscuitOS_root, node2.id, &node2);
//	radix_tree_insert(&BiscuitOS_root, node3.id, &node3);
//	radix_tree_insert(&BiscuitOS_root, node4.id, &node4);

	return 0;
	/* Iterate over Radix-tree */
	radix_tree_for_each_slot(slot, &BiscuitOS_root, &iter, 0)
		printf("Index: %#lx\n", iter.index);

	/* search struct node by id */
	np = radix_tree_lookup(&BiscuitOS_root, 0x321FE);
	BUG_ON(!np);
	printf("Radix: %s ID: %lx\n", np->name, np->id);

	/* Delete a node from radix-tree */
	radix_tree_delete(&BiscuitOS_root, node0.id);
	radix_tree_delete(&BiscuitOS_root, node1.id);
	radix_tree_delete(&BiscuitOS_root, node2.id);

	return 0;
}
