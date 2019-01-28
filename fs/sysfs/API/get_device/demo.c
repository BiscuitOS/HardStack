/*
 * sysfs: get_device
 *
 * struct device *get_device(struct device *dev)
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

static struct device BiscuitOS_bus = {
    .init_name = "BiscuitOS",
};

static int __init Demo_init(void)
{
    struct device *dev;
    int err;

    printk("Demo Procedure Entence...\n");

    /* Register a device */
    err = device_register(&BiscuitOS_bus);
    if (err) {
        printk("Unable to register device.\n");
        return -EINVAL;
    }

    /* Increase the reference of device */
    dev = get_device(&BiscuitOS_bus);
    if (dev)
        printk("Increase the reference of %s\n", dev_name(dev));

    /* Decrease the reference of device */
    put_device(dev);

    return 0;
}
device_initcall(Demo_init);
