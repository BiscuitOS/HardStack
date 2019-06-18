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
	unsigned long bitmap[] = { 0xfff, 0x20 };
	unsigned long pos;

	/* Find first zero bit */
	pos = find_first_zero_bit(bitmap, 128);
	printk("Find fist zero bit position: %ld\n", pos);

	return 0;
}
device_initcall(bitmap_demo_init);
