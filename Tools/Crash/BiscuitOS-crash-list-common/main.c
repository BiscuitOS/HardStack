/*
 * BiscuitOS CRASH list Common
 *
 * (C) 2021.06.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
/* header of list */
#include <linux/list.h>

/* Link structure */
struct BiscuitOS_node {
    const char *name;
    struct list_head list;
    struct BiscuitOS_node *next;
};

/* Declaration and implement a bindirect-list */
LIST_HEAD(BiscuitOS_list);

/* Initialize a group node structure */
static struct BiscuitOS_node node0 = { .name = "BiscuitOS-node0", };
static struct BiscuitOS_node node1 = { .name = "BiscuitOS-node1", };
static struct BiscuitOS_node node2 = { .name = "BiscuitOS-node2", };
static struct BiscuitOS_node node3 = { .name = "BiscuitOS-node3", };
static struct BiscuitOS_node node4 = { .name = "BiscuitOS-node4", };
static struct BiscuitOS_node node5 = { .name = "BiscuitOS-node5", };
static struct BiscuitOS_node node6 = { .name = "BiscuitOS-node6", };

/* Write */
static ssize_t BiscuitOS_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	char *p = NULL;
	struct BiscuitOS_node *np;

	/* add a new entry on special entry */
	list_add_tail(&node0.list, &BiscuitOS_list);
	list_add_tail(&node1.list, &BiscuitOS_list);
	list_add_tail(&node2.list, &BiscuitOS_list);
	list_add_tail(&node3.list, &BiscuitOS_list);
	list_add_tail(&node4.list, &BiscuitOS_list);
	list_add_tail(&node5.list, &BiscuitOS_list);
	list_add_tail(&node6.list, &BiscuitOS_list);

	/* private link */
	node0.next = &node1;
	node1.next = &node2;
	node2.next = &node3;
	node3.next = &node4;
	node4.next = &node5;
	node5.next = &node6;
	node6.next = NULL;

	printk("BiscuitOS-list:\n");
	/* Traverser all node on bindirect-list */
	list_for_each_entry(np, &BiscuitOS_list, list)
		printk("%#lx %s\n", np, np->name);

	/* Tigger PANIC */
	*p = 88;

	return len;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner	= THIS_MODULE,
	.write	= BiscuitOS_write,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "BiscuitOS",
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS CRASH Tools");
