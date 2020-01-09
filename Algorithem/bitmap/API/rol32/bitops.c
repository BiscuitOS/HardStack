/*
 * Bitops
 *
 * (C) 2020.01.09 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bitops.h>

/* Module initialize entry */
static int __init Demo_init(void)
{
	__u32 word = 0x12345678;
	__u32 data;
	
	data = rol32(word, 4);
	printk("Default: %#x shift  4 -> %#x\n", word, data);
	data = rol32(word, 8);
	printk("Default: %#x shift  8 -> %#x\n", word, data);
	data = rol32(word, 16);
	printk("Default: %#x shift 16 -> %#x\n", word, data);
	data = rol32(word, 24);
	printk("Default: %#x shift 24 -> %#x\n", word, data);
	data = rol32(word, 28);
	printk("Default: %#x shift 28 -> %#x\n", word, data);

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
MODULE_DESCRIPTION("Bitops Device driver");
