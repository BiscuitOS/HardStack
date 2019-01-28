/*
 * sysfs: bus_for_each_drv
 *
 * int bus_for_each_drv(struct bus_type *bus, struct device_driver *start,
 *          void *data, int (*fn)(struct device_driver *, void *))
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
#include <linux/platform_device.h>

/* print driver name */
static int __demo_device_attach(struct device_driver *drv, void *data)
{
    if (drv->name)
        printk("%s\n", drv->name);

    /* If traverse devices, must return 0 */
    return 0;
}

static int __init Demo_init(void)
{
    printk("Demo Procedure Entence...\n");

    /* Traverse all driver on special bus */
    bus_for_each_drv(&platform_bus_type, NULL, NULL, __demo_device_attach);

    return 0;
}
device_initcall(Demo_init);
