/*
 * Module Project
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "linux/demo.h"

int bs_show(void)
{
	printk("Hello BiscuitOS\n");

	return 0;
}
