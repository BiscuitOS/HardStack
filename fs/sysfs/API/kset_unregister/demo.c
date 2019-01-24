/*
 * sysfs: kset_unregister
 *
 * void kset_unregister(struct kset *k)
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
#include <linux/kobject.h>

/*
 * Kser and Kobject
 *
 * +--------+         +---------+         +---------+         +---------+
 * |        |         |         |         |         |         |         |
 * |  Kset  |         | Kobject |         | Kobject |         | Kobject |
 * |  list  |<------->|   entry |<------->|   entry |<------->|   entry |
 * |        |         |         |         |         |         |         |
 * +--------+         +---------+         +---------+         +---------+
 */

/* /sys/devices directory */
extern struct kset *devices_kset;

/* attribute show */
static ssize_t demo_show(struct kobject *obj, struct attribute *attr,
                                    char *buf)
{
    return 0;
}

/* attribute store */
static ssize_t demo_store(struct kobject *obj, struct attribute *attr,
                            const char *buf, size_t count)
{
    return 0;
}

/* sysfs operation */
static const struct sysfs_ops demo_attr_ops = {
    .show  = demo_show,
    .store = demo_store,
};

/* ktype */
static struct kobj_type BiscuitOS_ktype = {
    .sysfs_ops = &demo_attr_ops,
};

static struct kset BiscuitOS_kset = {
    .kobj = {
        .ktype = &BiscuitOS_ktype,
    },
};

static int __init Demo_init(void)
{
    struct kobject *kobj;

    printk("Demo Procedure Entence...\n");

    /* set kobject name */
    kobject_set_name(&BiscuitOS_kset.kobj, "BiscuitOS");
    /* set up kset for current kset */
    BiscuitOS_kset.kobj.kset = devices_kset;
    /* Register kset inot sysfs */
    kset_register(&BiscuitOS_kset);

    /* find kobject via kset */
    kobj = kset_find_obj(devices_kset, "BiscuitOS");
    if (kobj)
        printk("%s kset has register.\n", kobj->name);

    /* Unregister kset */
    kset_unregister(&BiscuitOS_kset);
    kobj = kset_find_obj(devices_kset, "BiscuitOS");
    if (!kobj)
        printk("BiscuitOS kset has unregister.\n");

    return 0;
}
device_initcall(Demo_init);
