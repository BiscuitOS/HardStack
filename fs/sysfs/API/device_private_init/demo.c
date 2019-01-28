/*
 * sysfs: device_private_init
 *
 * int device_private_init(struct device *dev)
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

extern int device_private_init(struct device *dev);

static struct device BiscuitOS_bus = {
    .init_name = "BiscuitOS",
};

static int __init Demo_init(void)
{
    int err;

    printk("Demo Procedure Entence...\n");

    /* Initialize 'device->p->klist_children' and 
                  'device->p->deferred_probe ' */
    device_private_init(&BiscuitOS_bus);

    /* Register a device */
    err = device_register(&BiscuitOS_bus);
    if (err) {
        printk("Unable to register device.\n");
        return -EINVAL;
    }

    return 0;
}
device_initcall(Demo_init);
