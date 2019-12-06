/*
 * BiscuitOS
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
#include <linux/fs.h>

/* Module initialize entry */
static int __init Demo_init(void)
{
	struct file_system_type *type;
	struct super_block *sb;
	struct inode *inode;

	/* obtian sb */
	type = get_fs_type("rootfs");
	sb = sget(type, NULL, set_anon_super, 0, NULL);
	if (!sb) {
		printk("Unable obtain sb.\n");
		return -ECHILD;
	} 

	/* obtain an inode */
	inode = new_inode(sb);

	/* Obtain a valid inode no */
	inode->i_ino = get_next_ino();
	inode_init_owner(inode, NULL, S_IFDIR);

	printk("INO: %ld\n", inode->i_ino);

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
MODULE_DESCRIPTION("BiscuitOS");
