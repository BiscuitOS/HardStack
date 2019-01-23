/*
 * sysfs: kobject_create_and_add
 *
 * struct kobject *kobject_create_and_add(const char *name,
 *                                            struct kobject *parent) 
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
#include <linux/list.h>

/* /sys/devices directory */
extern struct kset *devices_kset;

static int __init Demo_init(void)
{
    struct kobject *BiscuitOS_kobject;
    struct kobject *kobj;

    printk("Demo Procedure Entence...\n");

    /* Create a kobject and parent is "devices" */
    BiscuitOS_kobject = kobject_create_and_add("BiscuitOS", 
                                                  &devices_kset->kobj);
    if (!BiscuitOS_kobject) {
        printk("Unable to create and add kobject.\n");
        return -EINVAL;
    }
    
    /* kset's parent */
    if (BiscuitOS_kobject->parent)
        printk("%s parent: %s\n", BiscuitOS_kobject->name, 
                                  BiscuitOS_kobject->parent->name);

    printk("kobject: state_in_sysfs:    %d\n", 
                          BiscuitOS_kobject->state_in_sysfs);
    printk("kobject: state_initialized: %d\n",
                          BiscuitOS_kobject->state_initialized);

    /* Traverse all kobject on /sys/devices/ */
    list_for_each_entry(kobj, &devices_kset->list, entry) {
        if (kobj->name)
            printk("%s\n", kobj->name);
    }

    return 0;
}
device_initcall(Demo_init);
