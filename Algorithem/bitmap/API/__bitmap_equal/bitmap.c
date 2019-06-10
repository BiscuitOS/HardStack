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
	unsigned long bitmap0 = 0x90911016;
	unsigned long bitmap1 = 0x90921016;
	int bits = 16;

	if (__bitmap_equal(&bitmap0, &bitmap1, bits))
		printk("bitmap0: %#lx - bitmap1: %#lx equal bits[0 - %d]\n", 
				bitmap0, bitmap1, bits);

	return 0;
}
device_initcall(bitmap_demo_init);
