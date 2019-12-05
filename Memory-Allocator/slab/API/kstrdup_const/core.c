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
#include <linux/slab.h>

static const char *name;

/* Module initialize entry */
static int __init Demo_init(void)
{
	name = kstrdup_const("BiscuitOS", GFP_KERNEL);
	if (!name) {
		printk("Non free memory\n");
		return -ENOMEM;
	}

	/* display */
	printk("NAME: %s\n", name);
	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	kfree_const(name);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Device driver");
