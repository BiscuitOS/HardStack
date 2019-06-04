/*
 * IDR Manual.
 *
 * (C) 2019.06.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* IDR/IDA */
#include <idr.h>

/* private node */
struct node {
	const char *name;
};

/* Root of IDR */
static DEFINE_IDR(BiscuitOS_idr);

/* node set */
static struct node node0 = { .name = "IDA" };
static struct node node1 = { .name = "IDB" };
static struct node node2 = { .name = "IDC" };

/* ID array */
static int idr_array[8];

int main()
{
	struct node *np;
	int id;

	/* preload for idr_alloc() */
	idr_preload(GFP_KERNEL);

	/* Allocate a id from IDR */
	idr_array[0] = idr_alloc_cyclic(&BiscuitOS_idr, &node0, 1, 
							INT_MAX, GFP_ATOMIC);	
	idr_array[1] = idr_alloc_cyclic(&BiscuitOS_idr, &node1, 1, 
							INT_MAX, GFP_ATOMIC);	
	idr_array[2] = idr_alloc_cyclic(&BiscuitOS_idr, &node2, 1, 
							INT_MAX, GFP_ATOMIC);	

	idr_for_each_entry(&BiscuitOS_idr, np, id)
		printf("%s's ID %d\n", np->name, id);

	/* end preload section started with idr_preload() */
	idr_preload_end();
	return 0;
}
