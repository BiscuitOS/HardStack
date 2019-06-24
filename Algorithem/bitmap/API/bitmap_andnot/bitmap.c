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
	unsigned long bitmap0 = 0xff00;
	unsigned long bitmap1 = 0xff;
	unsigned long bitmap;

	/* ANDNOT operation */
	bitmap_andnot(&bitmap, &bitmap0, &bitmap1, 12);
	printk("%#lx ANDNOT %#lx = %#lx\n", bitmap0, bitmap1, bitmap);

	return 0;
}
device_initcall(bitmap_demo_init);
