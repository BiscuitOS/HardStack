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

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	printk("Hello BiscuitOS on kernel.\n");

	return 0;
}

device_initcall(BiscuitOS_init);
