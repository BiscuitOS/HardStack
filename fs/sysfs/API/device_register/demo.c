/*
 * sysfs: device_register
 *
 * int device_register(struct device *dev)
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
    int err;

    printk("Demo Procedure Entence...\n");

    err = device_register(&BiscuitOS_bus);
    if (err) {
        printk("Unable to register device.\n");
        return -EINVAL;
    }

    return 0;
}
device_initcall(Demo_init);
