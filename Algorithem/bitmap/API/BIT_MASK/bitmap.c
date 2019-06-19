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
	printk("BIT_MASK(0):  %#lx\n", BIT_MASK(0));
	printk("BIT_MASK(1):  %#lx\n", BIT_MASK(1));
	printk("BIT_MASK(3):  %#lx\n", BIT_MASK(3));
	printk("BIT_MASK(5):  %#lx\n", BIT_MASK(5));
	printk("BIT_MASK(17): %#lx\n", BIT_MASK(17));
	printk("BIT_MASK(24): %#lx\n", BIT_MASK(24));
	printk("BIT_MASK(32): %#lx\n", BIT_MASK(32));

	return 0;
}
device_initcall(bitmap_demo_init);
