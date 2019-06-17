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
	unsigned long bitmap[] = {0x12345678, 0x234edaff};
	unsigned int array[2];

	/* Cover bitmap into 32-bit array */
	bitmap_to_arr32(array, bitmap, 64);
	printk("Bitmap: %#x-%x\n", array[1], array[0]);

	return 0;
}
device_initcall(bitmap_demo_init);
