/*
 * IDR.
 *
 * (C) 2019.06.04 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>

/* header of radix-tree */
#include <linux/idr.h>

/* Root of IDR */
static DEFINE_IDR(BiscuitOS_idr);

static __init int idr_demo_init(void)
{
	struct radix_tree_iter iter;
	int id = 8;

	radix_tree_iter_init(&iter, id);
	idr_get_free(&BiscuitOS_idr.idr_rt, &iter, GFP_ATOMIC, INT_MAX);
	printk("INDEX: %#lx NEXT: %#lx\n", iter.index, iter.next_index);

	return 0;
}
device_initcall(idr_demo_init);
