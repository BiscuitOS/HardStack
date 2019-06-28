/*
 * Xarray.
 *
 * (C) 2019.06.06 <buddy.zhang@aliyun.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <stdio.h>
#include <stdlib.h>

/* Header of XArray */
#include <xarray.h>

/* Private */
struct node {
	char *name;
	unsigned long id;
};

/* static node declare and initialize */
static struct node node0 = { .name = "XA", .id = 0x455 };
static struct node node1 = { .name = "XB", .id = 0x15 };
static struct node node2 = { .name = "XB", .id = 0x375 };

/* Declare and implement XArray */
static DEFINE_XARRAY(BiscuitOS_xa);

int main()
{
	struct node *np;
	unsigned long index;

	/* Load xa entry */
	xa_store(&BiscuitOS_xa, node0.id, &node0, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node1.id, &node1, GFP_KERNEL);
	xa_store(&BiscuitOS_xa, node2.id, &node2, GFP_KERNEL);

	index = 0x15;
	np = xa_find(&BiscuitOS_xa, &index, ULONG_MAX, XA_PRESENT);
	printf("%s\n", np->name);

	/* xa_erase */
	xa_erase(&BiscuitOS_xa, node0.id);
	
	return 0;
}
