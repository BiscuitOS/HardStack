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
	printk("BIT_ULL_WORD(0):    %d\n", BIT_ULL_WORD(0));
	printk("BIT_ULL_WORD(31):   %d\n", BIT_ULL_WORD(31));
	printk("BIT_ULL_WORD(32):   %d\n", BIT_ULL_WORD(32));
	printk("BIT_ULL_WORD(64):   %d\n", BIT_ULL_WORD(64));
	printk("BIT_ULL_WORD(128):  %d\n", BIT_ULL_WORD(128));
	
	return 0;
}
device_initcall(bitmap_demo_init);
