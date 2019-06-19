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
	printk("BIT_ULL_MASK(0):  %#llx\n", BIT_ULL_MASK(0));
	printk("BIT_ULL_MASK(31): %#llx\n", BIT_ULL_MASK(31));
	printk("BIT_ULL_MASK(32): %#llx\n", BIT_ULL_MASK(32));
	printk("BIT_ULL_MASK(62): %#llx\n", BIT_ULL_MASK(62));
	printk("BIT_ULL_MASK(63): %#llx\n", BIT_ULL_MASK(63));
	
	return 0;
}
device_initcall(bitmap_demo_init);
