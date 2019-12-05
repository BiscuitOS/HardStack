/*
 * Kobject: kobject_create_and_add
 *
 * (C) 2019.11.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/rbtree.h>
#include <linux/rbtree_augmented.h>

static struct kobject *kobj;
extern struct kernfs_node *sysfs_root_kn;

#define rb_to_kn(X) rb_entry((X), struct kernfs_node, rb)

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct kernfs_node *kn, *pos;
	struct rb_root *rbroot;
	struct rb_node *np;

	kobj = kobject_create_and_add("BiscuitOS", NULL);
	if (!kobj)
		return -ENOMEM;

	kn = kobj->sd;
	printk("Kobj: %s\n", kobject_name(kobj));

	/* parent rbtree */
	rbroot = &kn->parent->dir.children;
	/* traverser all knode on rbtree */
	for (np = rb_first_postorder(rbroot); np;
				np = rb_next_postorder(np)) {
		pos = rb_to_kn(np);
		printk("NAME: %s\n", pos->name);
	}

	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	kobject_put(kobj);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS expamle Device driver");
