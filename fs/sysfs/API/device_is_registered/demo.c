/*
 * sysfs: device_is_registered
 *
 * static inline int device_is_registered(struct device *dev)
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

/* BiscuitOS device */
static struct device BiscuitOS_dev = {
   .init_name = "BiscuitOS",
};

static int __init Demo_init(void)
{
    printk("Demo Procedure Entence...\n");

    /* Detect and indicate whether device has registered */
    if (!device_is_registered(&BiscuitOS_dev))
        printk("%s doesn't register\n", dev_name(&BiscuitOS_dev));

    /* Register device */
    device_register(&BiscuitOS_dev);

    /* Detect and indicate whether device has registered */
    if (device_is_registered(&BiscuitOS_dev))
        printk("%s has registered\n", dev_name(&BiscuitOS_dev));

    return 0;
}
device_initcall(Demo_init);
