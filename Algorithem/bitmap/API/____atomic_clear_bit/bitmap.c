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
	unsigned long bit = 0xff;

	/* Clear a bit with atomic */
	____atomic_clear_bit(1, &bit);
	printk("bit: %#lx\n", bit);

	return 0;
}
device_initcall(bitmap_demo_init);
