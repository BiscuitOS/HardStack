/*
 * sysfs: kset_find_obj
 *
 * struct kobject *kset_find_obj(struct kset *kset, const char *name)
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

static int __init Demo_init(void)
{
    struct kobject *kobj;

    printk("Demo Procedure Entence...\n");

    /* search for "platform" kobject in kset */
    kobj = kset_find_obj(devices_kset, "platform"); 
    if (!kobj) {
        printk("Unable to find platform kobject\n");
        return -EINVAL;
    }
    printk("%s kobject found.\n", kobj->name);

    return 0;
}
device_initcall(Demo_init);
