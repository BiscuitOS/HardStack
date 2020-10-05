/*
 * Syscore suspend/resume
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

#include <linux/syscore_ops.h>

static int BiscuitOS_suspend(void)
{
	printk("Trigger suspend on BiscuitOS\n");
	return 0;
}

static void BiscuitOS_resume(void)
{
	printk("Trigger resume on BiscuitOS.\n");
}

static struct syscore_ops BiscuitOS_syscore = {
	.suspend = BiscuitOS_suspend,
	.resume  = BiscuitOS_resume,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	printk("Hello modules on BiscuitOS\n");

	register_syscore_ops(&BiscuitOS_syscore);	

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	unregister_syscore_ops(&BiscuitOS_syscore);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
