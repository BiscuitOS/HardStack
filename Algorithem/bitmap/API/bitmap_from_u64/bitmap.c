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

	unsigned long bitmap[2] = {0};
	u64 map = 0x123456789abcdef;

	/* Cover u64 to bitmap */
	bitmap_from_u64(bitmap, map);
	printk("%#llx cover to [0]%#lx [1]%#lx\n", map, 
						bitmap[0], bitmap[1]);

	return 0;
}
device_initcall(bitmap_demo_init);
