/*
 * sysfs: sysfs_remove_group
 *
 * void sysfs_remove_group(struct kobject *kobj,
 *                       const struct attribute_group *grp)
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

/*
 * Relation between kset and kobject
 *
 *
 *                         /sys/devices
 *                        +----------+
 *                        |          |
 *                        |   kset   |
 *                        |     list |<-----------------------o
 *                        +----------+                        |
 *                        | kobject  |                        |
 *  o-------------------->+----------+                        |
 *  |                                                         |
 *  |                                                         |
 *  |                                                         |
 *  |   /sys/devices/dev  /sys/devices/cpu  /sys/devices/soc  |   
 *  |   +---------+       +---------+       +---------+       |
 *  |   |         |       |         |       |         |       |
 *  |   | kobject |       | kobject |       | kobject |       |
 *  |   |   entry |<----->|   entry |<----->|   entry |<------o
 *  |   |  parent |--o    |  parent |--o    |  parent |--o     
 *  |   |         |  |    |         |  |    |         |  |    
 *  |   +---------+  |    +---------+  |    +---------+  |    
 *  |                V                 V                 |
 *  o----------------o-----------------o-----------------o
 *
 */

/* /sys/devices directory */
extern struct kset *devices_kset;

/* attribute show */
static ssize_t demo0_show(struct device *dev, struct device_attribute *attr,
                                       char *buf)
{
    printk("Demo0 attribute show.....\n");
    return 0;
}

/* attribute store */
static ssize_t demo0_store(struct device *dev, struct device_attribute *attr,
                               const char *buf, size_t count)
{
    printk("Demo0 attribute store....\n");
    return count;
}

/* attribute show */
static ssize_t demo1_show(struct device *dev, struct device_attribute *attr,
                                        char *buf)
{
    printk("Demo1 attribute show....\n");
    return 0;
}

/* attribute store */
static ssize_t demo1_store(struct device *dev, struct device_attribute *attr,
                               const char *buf, size_t count)
{
    printk("Demo1 attribute store....\n");
    return count;
}

/* Establish two attribute which contains show() and store() */
static struct device_attribute demo0_attr =
    __ATTR(demo0, S_IRUGO | S_IWUSR, demo0_show, demo0_store);
static struct device_attribute demo1_attr =
    __ATTR(demo1, S_IRUGO | S_IWUSR, demo1_show, demo1_store);

/* Establish a attribute array to contains all device attribute */
static struct attribute *Demo_attrs[] = {
    &demo0_attr.attr,
    &demo1_attr.attr,
    NULL
};

/* Establish a attribute group */
static const struct attribute_group Demo_attr_group = {
    .attrs = Demo_attrs
};

static int __init Demo_init(void)
{
    struct kobject *kobj;

    printk("Demo Procedure Entence...\n");

    /* Create a kobject and add sysfs: /sys/devices/BiscuitOS */
    kobj = kobject_create_and_add("BiscuitOS", &devices_kset->kobj);
    if (!kobj) {
        printk("Unable to create a kobject!\n");
        return -EINVAL;
    }

    /* Given a directory kobject, and create an attribute group */
    sysfs_create_group(kobj, &Demo_attr_group);

    /* Remove attribute group from a given directory */
    sysfs_remove_group(kobj, &Demo_attr_group);

    return 0;
}
device_initcall(Demo_init);
