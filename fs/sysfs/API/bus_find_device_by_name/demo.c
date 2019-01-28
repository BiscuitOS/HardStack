/*
 * sysfs: bus_find_device_by_name
 *
 * struct device *bus_find_device_by_name(struct bus_type *bus,
 *           struct device *start, const char *name)
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

/*
 * Ref: Bus and device
 *
 *
 *                  +----------------+
 *                  |                |
 * +----------+     | subsys_private |
 * |          |     |  klist_devices-|------------------------------o
 * | bus_type |     |                |                              |
 * |       p -|---->+----------------+                              |
 * |          |                                                     |
 * +----------+                                                     |
 *                                                                  |
 *                                                                  |
 *     +--------+                +--------+                         |
 *     |        |                |        |                         |
 *     | device |                | device |                         |
 *     |     p -|-------o        |     p -|-------o                 |
 *     |        |       |        |        |       |                 |
 *     +--------+       |        +--------+       |                 |
 *                      |                         |                 |
 * +----------------+<--o    +----------------+<--o                 |
 * |                |        |                |                     |
 * | device_private |        | device_private |                     |
 * |      knode_bus |<------>|      knode_bus |<-----.......<-------o
 * |                |        |                |
 * +----------------+        +----------------+
 *
 */

static int __init Demo_init(void)
{
    struct device *dev;

    printk("Demo Procedure Entence...\n");

    /* Find a device by name on special bus */
    dev = bus_find_device_by_name(&platform_bus_type, NULL, "pmu.14");
    if (dev)
        printk("Find device %s\n", dev_name(dev));

    return 0;
}
device_initcall(Demo_init);
