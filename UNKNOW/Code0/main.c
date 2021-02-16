/*
 * BiscuitOS Kernel BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>

#define BISCUITOS_CLASS_NAME		"BiscuitOS"

static int BiscuitOS_online(struct device *dev)
{
	printk("BiscuitOS online.\n");
	return 0;
}

static int BiscuitOS_offline(struct device *dev)
{
	printk("BiscuitOS_offline.\n");
	return 0;
}

static ssize_t BiscuitOS_block_size_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lx\n", (unsigned long)0x100000);
}

static DEVICE_ATTR_RO(BiscuitOS_block_size);

static struct attribute *BiscuitOS_root_attrs[] = {
	&dev_attr_BiscuitOS_block_size.attr,
	NULL,
};

static struct attribute_group BiscuitOS_attr_group = {
	.attrs = BiscuitOS_root_attrs,
};

static const struct attribute_group *BiscuitOS_attr_groups[] = {
	&BiscuitOS_attr_group,
	NULL,
};

static struct bus_type BiscuitOS_subsys = {
	.name = BISCUITOS_CLASS_NAME,
	.dev_name = BISCUITOS_CLASS_NAME,
	.online = BiscuitOS_online,
	.offline = BiscuitOS_offline,
};

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	printk("Hello BiscuitOS on kernel.\n");

	subsys_system_register(&BiscuitOS_subsys, BiscuitOS_attr_groups);

	return 0;
}

device_initcall(BiscuitOS_init);
