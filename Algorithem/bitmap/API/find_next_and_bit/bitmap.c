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
	unsigned long bitmap1 = 0xfff00;
	unsigned long bitmap2 = 0x1ff;
	unsigned long pos;

	/* Find first set bit */
	pos = find_next_and_bit(&bitmap1, &bitmap2, 32, 0);
	printk("%#lx find first set bit through bit 0 to bit 32: %ld\n", 
						bitmap1, pos);

	return 0;
}
device_initcall(bitmap_demo_init);
