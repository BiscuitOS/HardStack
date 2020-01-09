/*
 * Device Driver
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define HASH_MIX(x, y, a)						\
	(								\
		x ^= (a),						\
		y ^= x, x = rol32(x, 7),				\
		x += y, y = rol32(y, 20),				\
		y *= 9							\
	)

/* Module initialize entry */
static int __init Demo_init(void)
{
	unsigned long x = 0, y = 0;
	unsigned long slot = 0x12345678;

	printk("Default: x %#lx y %#lx slot: %#lx\n", x, y, slot);
	HASH_MIX(x, y, slot);
	printk("HashMix: x %#lx y %#lx slot: %#lx\n", x, y, slot);

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Device driver");
