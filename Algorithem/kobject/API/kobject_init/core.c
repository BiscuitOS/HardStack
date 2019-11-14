/*
 * Kobject
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
#include <linux/slab.h>

struct Demo_dev {
	struct kobject	kobj;
	int dev_size;
};

struct Demo_sysfs_entry {
	struct attribute attr;
	ssize_t (*show)(struct Demo_dev *, char *);
	ssize_t (*store)(struct Demo_dev *, const char *, size_t);
};

static struct Demo_dev *bdev;

/* free */
static void Demo_free(struct kobject *ko)
{
	struct Demo_dev *bdev = container_of(ko, struct Demo_dev, kobj);

	kfree(bdev);
	bdev = NULL;
}

static ssize_t Demo_attr_show(struct kobject *kobj, 
				struct attribute *attr, char *page)
{
	return 0;
}

static ssize_t Demo_attr_store(struct kobject *kobj,
		struct attribute *attr, const char *page, size_t length)
{
	return 0;
}

static const struct sysfs_ops Demo_sysfs_ops = {
	.show	= Demo_attr_show,
	.store	= Demo_attr_store,
};

static ssize_t size_show(struct Demo_dev *bdev, char *page)
{
	return sprintf(page, "%d\n", (int)bdev->dev_size);
}

static ssize_t size_store(struct Demo_dev *bdev, const char *buf, size_t len)
{
	sscanf(buf, "%d", &bdev->dev_size);
	return len;
}

static struct Demo_sysfs_entry Demo_size =
__ATTR(BiscuitOS_size, S_IRUGO | S_IWUSR, size_show, size_store);

static struct attribute *Demo_default_attrs [] = {
	&Demo_size.attr,
	NULL,
};

/* kset */
static struct kobj_type Demo_ktype = {
	.release	= Demo_free,
	.sysfs_ops	= &Demo_sysfs_ops,
	.default_attrs	= Demo_default_attrs,
};

/* Module initialize entry */
static int __init Demo_init(void)
{
	int ret;

	bdev = kzalloc(sizeof(*bdev), GFP_KERNEL);
	if (!bdev) {
		printk("Error: kzalloc()\n");
		ret = -ENOMEM;
		goto err_all;
	}

	kobject_init(&bdev->kobj, &Demo_ktype);

	return 0;

err_all:
	return ret;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	if (bdev)
		kfree(bdev);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Kobject Device driver");
