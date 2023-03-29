// SPDX-License-Identifier: GPL-2.0
/*
 * Module Project
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "linux/demo.h"

int BiscuitOS_show(void)
{
	printk("Hello BiscuitOS\n");

	return 0;
}
