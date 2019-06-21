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
	const char *buf = "a2100347";
	unsigned long bitmap[10];

	bitmap_parse(buf, 32, bitmap, 32);
	printk("%s bitmap: %#lx\n", buf, bitmap[0]);

	return 0;
}
device_initcall(bitmap_demo_init);
