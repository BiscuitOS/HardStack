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
	unsigned long bitmap = 0x45003;

	/* Detect bitmap full? */
	if (bitmap_full(&bitmap, 2))
		printk("%#lx is full through bit 0 to 1.\n", bitmap);

	if (!bitmap_full(&bitmap, 3))
		printk("%#lx is not full through bit 0 to 2.\n", bitmap);

	return 0;
}
device_initcall(bitmap_demo_init);
