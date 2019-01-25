/*
 * sysfs: sysfs_create_dir
 *
 * int sysfs_create_dir(struct kobject *kobj)
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

static const struct kobj_type BiscuitOS_ktype;

static struct kobject BiscuitOS_kobj = 
{
    .name  = "BiscuitOS",
};

static int __init Demo_init(void)
{
    printk("Demo Procedure Entence...\n");

    /* Initialize kobject */
    kobject_init(&BiscuitOS_kobj, &BiscuitOS_ktype);
    
    /* Set kobject parent, points to devices_kset */
    BiscuitOS_kobj.parent = &devices_kset->kobj;

    /* create dir */
    sysfs_create_dir(&BiscuitOS_kobj);

    return 0;
}
device_initcall(Demo_init);
