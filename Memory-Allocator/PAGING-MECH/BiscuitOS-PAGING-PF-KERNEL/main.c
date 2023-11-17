// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel 
 *
 * (C) 2023.11.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

static int __init BiscuitOS_init(void)
{
	void *mem = NULL;

	/* Write Ops, Trigger #PF */
	*(char *)mem = 'B';

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
