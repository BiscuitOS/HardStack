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
	unsigned long bitmap1 = 0xffff00f1;
	unsigned int pos;

	/* Find position for first zero area. */
	pos = bitmap_find_next_zero_area_off(&bitmap1, 32, 0, 3, 1, 0);
	printk("Find %#lx first zero area postion is: %d\n", bitmap1, pos);

	return 0;
}
device_initcall(bitmap_demo_init);
