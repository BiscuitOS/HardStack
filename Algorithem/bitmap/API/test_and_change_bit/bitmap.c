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
	unsigned long bitmap = 0x1230068;
	unsigned long old = bitmap;
	int ret;

	/* set bit */
	set_bit(9, &bitmap);
	printk("set_bit(9, %#lx): %#lx\n", old, bitmap);

	old = bitmap;
	/* clear bit */
	clear_bit(9, &bitmap);
	printk("clear_bit(9, %#lx): %#lx\n", old, bitmap);

	old = bitmap;
	/* Change bit */
	change_bit(9, &bitmap);
	printk("change_bit(9, %#lx): %#lx\n", old, bitmap);

	old = bitmap;
	/* Set bit and return original value */
	ret = test_and_set_bit(9, &bitmap);
	printk("test_and_set_bit(9, %#lx): %#lx (origin: %d)\n", old, 
							bitmap, ret);

	old = bitmap;
	/* Clear bit and return original value */
	ret = test_and_clear_bit(9, &bitmap);
	printk("test_and_clear_bit(9, %#lx): %#lx (origin: %d)\n", old,
							bitmap, ret);

	old = bitmap;
	/* Change bit and return original value */
	ret = test_and_change_bit(9, &bitmap);
	printk("test_and_change_bit(9, %#lx): %#lx (origin: %d)\n",
					old, bitmap, ret);
	

	return 0;
}
device_initcall(bitmap_demo_init);
