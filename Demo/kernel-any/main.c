// SPDX-License-Identifier: GPL-2.0
/*
 * BiscuitOS Kernel Anywhere BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>

int __init BiscuitOS_Running(void)
{
	printk("Hello BiscuitOS anywhere on kernel.\n");

	return 0;
}
