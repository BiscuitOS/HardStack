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
	__u8 word = 0xCD;
	__u8 data;
	
	data = ror8(word, 1);
	printk("Default: %#hx shift  1 -> %#x\n", word, data);
	data = ror8(word, 2);
	printk("Default: %#hx shift  2 -> %#x\n", word, data);
	data = ror8(word, 4);
	printk("Default: %#hx shift  4 -> %#x\n", word, data);
	data = ror8(word, 5);
	printk("Default: %#hx shift  5 -> %#x\n", word, data);
	data = ror8(word, 6);
	printk("Default: %#hx shift  6 -> %#x\n", word, data);

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
