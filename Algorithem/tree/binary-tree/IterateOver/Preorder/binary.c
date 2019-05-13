/*
 * Binary-Tree.
 *
 * (C) 2019.05.12 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* binary-tree node */
struct binary_node {
	int idx;
	struct binary_node *left;
	struct binary_node *right;
};

/* Perfect Binary Tree  
 *                               (200)
 *                                 |
 *                 o---------------+---------------o
 *                 |                               |
 *               (143)                           (876)  
 *                 |                               |
 *          o------+-----o                  o------+-----o
 *          |            |                  |            |
 *        (754)        (386)              (486)        (740)
 *          |            |                  |            |
 *      o---+---o    o---+---o          o---+---o    o---+---o
 *      |       |    |       |          |       |    |       |
 *     (6)     (3)  (7)     (9)        (8)     (2)  (1)     (5)
 */
static int Perfect_BinaryTree_data[] = { 
                                  200, 143, 754, 6, -1, -1, 3, -1, -1,
                                  386, 7, -1, -1, 9, -1, -1, 876, 486, 
                                  8, -1, -1, 2, -1, -1, 740, 1, -1, -1, 
                                  5, -1, -1 };

static int counter = 0;
static int *BinaryTree_data = Perfect_BinaryTree_data;

/* Preorder Create Binary-tree */
static struct binary_node *Preorder_Create_BinaryTree(struct binary_node *node)
{
	int ch = BinaryTree_data[counter++];

	/* input from terminal */
	if (ch == -1) {
		return NULL;
	} else {
		node = 
		   (struct binary_node *)malloc(sizeof(struct binary_node));	
		node->idx = ch;

		/* Create left child */
		node->left  = Preorder_Create_BinaryTree(node->left);
		/* Create right child */
		node->right = Preorder_Create_BinaryTree(node->right);
		return node;
	}
}

/* Pre-Traverse Binary-Tree */
static void Preorder_Traverse_BinaryTree(struct binary_node *node)
{
	if (node == NULL) {
		return;
	} else {
		printf("%d ", node->idx);
		/* Traverse left child */
		Preorder_Traverse_BinaryTree(node->left);
		/* Traverse right child */
		Preorder_Traverse_BinaryTree(node->right);
	}
}

/* Post-Free BinaryTree */
static void Postorder_Free_BinaryTree(struct binary_node *node)
{
	if (node == NULL) {
		return;
	} else {
		Postorder_Free_BinaryTree(node->left);
		Postorder_Free_BinaryTree(node->right);
		free(node);
		node = NULL;
	}
}

int main()
{
	/* Define binary-tree root */
	struct binary_node *BiscuitOS_root;
	
	printf("Preorder Create BinaryTree\n");
	BiscuitOS_root = Preorder_Create_BinaryTree(BiscuitOS_root);

	/* Preoder traverse binary-tree */
	printf("Preorder Traverse Binary-Tree:\n");
	Preorder_Traverse_BinaryTree(BiscuitOS_root);
	printf("\n");

	/* Postorder free binary-tree */
	Postorder_Free_BinaryTree(BiscuitOS_root);

	return 0;
}
