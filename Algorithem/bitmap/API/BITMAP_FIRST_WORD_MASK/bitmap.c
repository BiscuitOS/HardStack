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
	printk("Bitmap(0):   %#lx\n", BITMAP_FIRST_WORD_MASK(0));
	printk("Bitmap(1):   %#lx\n", BITMAP_FIRST_WORD_MASK(1));
	printk("Bitmap(2):   %#lx\n", BITMAP_FIRST_WORD_MASK(2));
	printk("Bitmap(3):   %#lx\n", BITMAP_FIRST_WORD_MASK(3));
	printk("Bitmap(4):   %#lx\n", BITMAP_FIRST_WORD_MASK(4));
	printk("Bitmap(5):   %#lx\n", BITMAP_FIRST_WORD_MASK(5));
	printk("Bitmap(6):   %#lx\n", BITMAP_FIRST_WORD_MASK(6));
	printk("Bitmap(7):   %#lx\n", BITMAP_FIRST_WORD_MASK(7));
	printk("Bitmap(8):   %#lx\n", BITMAP_FIRST_WORD_MASK(8));
	printk("Bitmap(10):  %#lx\n", BITMAP_FIRST_WORD_MASK(10));
	printk("Bitmap(12):  %#lx\n", BITMAP_FIRST_WORD_MASK(12));
	printk("Bitmap(16):  %#lx\n", BITMAP_FIRST_WORD_MASK(16));
	printk("Bitmap(18):  %#lx\n", BITMAP_FIRST_WORD_MASK(18));
	printk("Bitmap(20):  %#lx\n", BITMAP_FIRST_WORD_MASK(20));
	printk("Bitmap(22):  %#lx\n", BITMAP_FIRST_WORD_MASK(22));
	printk("Bitmap(24):  %#lx\n", BITMAP_FIRST_WORD_MASK(24));
	printk("Bitmap(26):  %#lx\n", BITMAP_FIRST_WORD_MASK(26));
	printk("Bitmap(27):  %#lx\n", BITMAP_FIRST_WORD_MASK(27));
	printk("Bitmap(28):  %#lx\n", BITMAP_FIRST_WORD_MASK(28));
	printk("Bitmap(29):  %#lx\n", BITMAP_FIRST_WORD_MASK(29));
	printk("Bitmap(30):  %#lx\n", BITMAP_FIRST_WORD_MASK(30));
	printk("Bitmap(31):  %#lx\n", BITMAP_FIRST_WORD_MASK(31));
	printk("Bitmap(31):  %#lx\n", BITMAP_FIRST_WORD_MASK(32));

	return 0;
}
device_initcall(bitmap_demo_init);
