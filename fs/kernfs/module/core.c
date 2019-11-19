/*
 * Device Driver
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "linux/bsfs.h"

static struct bsfs_root *bsfs_root;

/* Module initialize entry */
static int __init Demo_init(void)
{
	bsfs_init();

	bsfs_root = bsfs_create_root(NULL, 
			BSFS_ROOT_EXTRA_OPEN_PERM_CHECK, NULL);

	if (IS_ERR(bsfs_root))
		return PTR_ERR(bsfs_root);

	printk("BSFilesystem Root install...\n");
	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Device driver");
