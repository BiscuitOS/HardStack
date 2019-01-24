/*
 * sysfs: sysfs_create_file
 *
 * int sysfs_create_file(struct kobject *kobj, const struct attribute *attr)
 *
 * (C) 2019.01.23 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/device.h>

/* /sys/devices directory */
extern struct kset *devices_kset;

static ssize_t store_demo(struct device *dev, struct device_attribute *attr,
                 const char *buf, size_t count)
{
    printk("attribute store entence.\n");

    return count;
}

static ssize_t show_demo(struct device *dev, struct device_attribute *attr,
                           char *buf)
{
    printk("attribute show entence.\n");

    return 0;
}

static struct device_attribute demo_attr =
    __ATTR(demo, S_IRUGO | S_IWUSR, show_demo, store_demo);

static int __init Demo_init(void)
{
    struct kobject *kobj;

    printk("Demo Procedure Entence...\n");

    /* Create a kobject under "/sys/devices/ "*/
    kobj = kobject_create_and_add("BiscuitOS", &devices_kset->kobj);

    /* Add a file into /sys/devices/BiscuitOS/ */
    sysfs_create_file(kobj, &demo_attr.attr);

    return 0;
}
device_initcall(Demo_init);
