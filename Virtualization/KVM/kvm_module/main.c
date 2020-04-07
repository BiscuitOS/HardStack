/*
 * KVM
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Module initialize entry */
static int __init kvm_init_bs(void)
{
	printk("BiscuitOS KVM\n");
	return 0;
}

/* Module exit entry */
static void __exit kvm_exit_bs(void)
{
}

module_init(kvm_init_bs);
module_exit(kvm_exit_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS KVM Project");
