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
	unsigned long bitmap1 = 0x90911010;
	unsigned long bitmap2 = bitmap1;

	/* fill bitmap */
	bitmap_fill(&bitmap1, 31);
	printk("%#lx fill bitmap: %#lx\n", bitmap2, bitmap1);

	return 0;
}
device_initcall(bitmap_demo_init);
