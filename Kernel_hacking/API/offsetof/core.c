/*
 * BiscuitOS
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
#include <linux/fs.h>
#include <linux/slab.h>

struct demo_node
{
	int size;
	short page;
	unsigned long offset;
	struct demo_node *next;
	char *name;
	unsigned int index;
};

/* Module initialize entry */
static int __init Demo_init(void)
{
	printk("demo_node->size:\t%d\n", offsetof(struct demo_node, size));
	printk("demo_node->page:\t%d\n", offsetof(struct demo_node, page));
	printk("demo_node->offset:\t%d\n", offsetof(struct demo_node, offset));
	printk("demo_node->next:\t%d\n", offsetof(struct demo_node, next));
	printk("demo_node->name:\t%d\n", offsetof(struct demo_node, name));
	printk("demo_node->index:\t%d\n", offsetof(struct demo_node, index));

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
MODULE_DESCRIPTION("BiscuitOS");
