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
	unsigned long bitmap0 = 0xff45;
	unsigned long bitmap1 = 0x7845;

	/* Check bitmap whether intersects */
	if (bitmap_intersects(&bitmap0, &bitmap1, 8))
		printk("%#lx intersect %#lx through 0 to 7.\n",
						bitmap0, bitmap1);

	return 0;
}
device_initcall(bitmap_demo_init);
