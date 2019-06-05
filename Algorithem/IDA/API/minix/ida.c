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

/* Root of IDR */
static DEFINE_IDA(BiscuitOS_ida);

static __init int ida_demo_init(void)
{
	int id;

	/* Allocate an unused ID */
	id = ida_alloc(&BiscuitOS_ida, GFP_KERNEL);

	printk("IDA-ID: %#x\n", id);

	/* Release an allocated ID */
	ida_free(&BiscuitOS_ida, id);

	return 0;
}
device_initcall(ida_demo_init);
