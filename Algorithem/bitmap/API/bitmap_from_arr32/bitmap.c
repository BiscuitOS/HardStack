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
	unsigned int array[] = { 0x12345678, 0x56456733 };
	unsigned long bitmap[2];

	/* Cover u32 array into bitmap */
	bitmap_from_arr32(bitmap, array, 64);
	printk("Bitmap: %#lx%lx\n", bitmap[1], bitmap[0]);

	return 0;
}
device_initcall(bitmap_demo_init);
