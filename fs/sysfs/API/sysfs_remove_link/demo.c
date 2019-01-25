/*
 * sysfs: sysfs_remove_link
 *
 * int sysfs_remove_link(struct kobject *kobj, const char *name)
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

    /* Create a soft-link named "parent": /sys/devices/BiscuitOS/parent */
    sysfs_create_link(kobj, &devices_kset->kobj, "parent");

    /* Remove a soft-link */
    sysfs_remove_link(kobj, "parent");

    return 0;
}
device_initcall(Demo_init);
