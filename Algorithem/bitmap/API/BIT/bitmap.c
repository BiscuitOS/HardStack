/*
 * Bitmap.
 *
 * (C) 2019.06.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

/* header of bitmap */
#include <linux/bitmap.h>

static __init int bitmap_demo_init(void)
{
	printk("BIT(0):   %#lx\n", BIT(0));
	printk("BIT(2):   %#lx\n", BIT(2));
	printk("BIT(30):  %#lx\n", BIT(30));
	printk("BIT(31):  %#lx\n", BIT(31));

	return 0;
}
device_initcall(bitmap_demo_init);
