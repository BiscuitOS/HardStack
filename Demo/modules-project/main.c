// SPDX-License-Identifier: GPL-2.0
/*
 * Module Project on BiscuitOS
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "linux/demo.h"

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	BiscuitOS_show();

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Module Project on BiscuitOS");
