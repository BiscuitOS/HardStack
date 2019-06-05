/*
 * IDA.
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

/* header of IDA/IDR */
#include <linux/idr.h>
#include <linux/radix-tree.h>

/* Root of IDA */
static DEFINE_IDA(BiscuitOS_ida);

static __init int ida_demo_init(void)
{
	int id;

	/* Allocate an unused ID */
	id = ida_simple_get(&BiscuitOS_ida, 0, INT_MAX, GFP_KERNEL);
	printk("IDA-ID: %#x\n", id);

	/* Release an ID */
	ida_simple_remove(&BiscuitOS_ida, id);

	/* Free all IDs */
	ida_destroy(&BiscuitOS_ida);

	return 0;
}
device_initcall(ida_demo_init);
