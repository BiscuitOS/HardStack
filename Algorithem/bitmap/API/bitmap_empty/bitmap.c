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
	unsigned long bitmap = 0xfff00;

	if (bitmap_empty(&bitmap, 8))
		printk("Bitmap: %#lx[0:8] is empty\n", bitmap);

	if (!bitmap_empty(&bitmap, 9))
		printk("Bitmap: %#lx[0:9] is not empty\n", bitmap);

	return 0;
}
device_initcall(bitmap_demo_init);
