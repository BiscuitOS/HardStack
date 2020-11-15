/*
 * BiscuitOS Kernel Anywhere BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

int __init BiscuitOS_Running(void)
{
	printk("Hello BiscuitOS anywhere on kernel.\n");

	return 0;
}
