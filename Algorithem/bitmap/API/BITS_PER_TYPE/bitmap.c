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
	printk("BITS_PER_TYPE(char):   %d\n", BITS_PER_TYPE(char));
	printk("BITS_PER_TYPE(short):  %d\n", BITS_PER_TYPE(short));
	printk("BITS_PER_TYPE(int):    %d\n", BITS_PER_TYPE(int));
	printk("BITS_PER_TYPE(long):   %d\n", BITS_PER_TYPE(long));

	return 0;
}
device_initcall(bitmap_demo_init);
