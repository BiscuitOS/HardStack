// SPDX-License-Identifier: GPL-2.0
/*
 * BiscuitOS Kernel BiscuitOS Code
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	printk("Hello BiscuitOS on kernel.\n");

	return 0;
}

device_initcall(BiscuitOS_init);
