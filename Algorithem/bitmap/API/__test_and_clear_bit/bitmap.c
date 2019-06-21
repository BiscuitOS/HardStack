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
	int ret;

	/* Clear a bit and return its old value */
	ret = __test_and_clear_bit(1, &bit);
	printk("%#lx before 1's bit is: %d\n", bit, ret);

	return 0;
}
device_initcall(bitmap_demo_init);
