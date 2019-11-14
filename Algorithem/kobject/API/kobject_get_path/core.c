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

static struct kobject *kobj;

/* Module initialize entry */
static int __init Demo_init(void)
{
	int ret;
	char *devpath = NULL;

	kobj = kobject_create_and_add("BiscuitOS", NULL);
	if (!kobj)
		return -ENOMEM;

	kobj = kobject_get(kobj);
	if (!kobj) {
		printk("Error: kobject_get\n");
		ret = -EINVAL;
		goto err_get;
	}

	/* Path */
	devpath = kobject_get_path(kobj, GFP_KERNEL);
	if (!devpath) {
		printk("Error: kobject_get_path()\n");
		ret = -ENOMEM;
		goto err_path;
	}
	printk("BiscuitOS Path: /sys%s\n", devpath);

	kobject_put(kobj);

	return 0;

err_path:
	kobject_put(kobj);
err_get:
	kobject_put(kobj);

	return ret;
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
MODULE_DESCRIPTION("BiscuitOS Kobject Device driver");
