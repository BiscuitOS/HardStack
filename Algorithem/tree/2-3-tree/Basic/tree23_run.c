/*
 * 2-3-Tree Manual.
 *
 * (C) 2019.05.20 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Head of 2-3 tree */
#include <tree23.h>

/* Root of Tree23 */
static struct tree23_root *BiscuitOS_root;

#define NUM_MAX		16
/* data */
static unsigned long Tree23_data[NUM_MAX] = { 23, 67, 344, 56, 12334, 56, 
				87, 568, 423, 987, 1, 876, 76542, 3245,
				8976, 90 };

int main()
{
	int i;

	/* creat 2-3 tree */
	BiscuitOS_root = tree23_root_init();

	for (i = 0; i < NUM_MAX; i++) {
		/* Insert node into 2-3 tree */
		tree23_insert(Tree23_data[i], BiscuitOS_root);
	}

	printf("Itervate over 2-3 Tree.\n");
	/* tree23 interate over */
	tree23_print(BiscuitOS_root->root);

	/* Erase */
	tree23_erase(423, BiscuitOS_root);
	tree23_erase(876, BiscuitOS_root);

	printf("Re- Itervate over 2-3 Tree\n");
	tree23_print(BiscuitOS_root->root);

	/* delete 2-3 tree */
	tree23_deltree(BiscuitOS_root);

	return 0;
}
