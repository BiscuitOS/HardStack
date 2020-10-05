/*
 * Reboot notifier
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* reboot notifier */
#include <linux/reboot.h>

static int BiscuitOS_reboot_notifier(struct notifier_block *notifier,
                                        	unsigned long val, void *v)
{
	printk("Trigger reboot on BiscuitOS.\n");
	return 0;
}

static struct notifier_block BiscuitOS_reboot = {
	.notifier_call = BiscuitOS_reboot_notifier,
	.priority = 0,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	printk("Hello modules on BiscuitOS\n");

	/* Register reboot notifier */
	register_reboot_notifier(&BiscuitOS_reboot);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	unregister_reboot_notifier(&BiscuitOS_reboot);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
