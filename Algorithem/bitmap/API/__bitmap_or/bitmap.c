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
	unsigned long dst;
	unsigned long bitmap1 = 0xffff0000;
	unsigned long bitmap2 = 0x0000fff1;

	__bitmap_or(&dst, &bitmap1, &bitmap2, 32);
	printk("DST: %#lx\n", dst);

	return 0;
}
device_initcall(bitmap_demo_init);
