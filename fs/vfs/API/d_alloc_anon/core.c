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
	struct dentry *dentry, *parent;
	struct qstr string;

	/* obtian sb */
	type = get_fs_type("rootfs");
	sb = sget(type, NULL, set_anon_super, 0, NULL);
	if (!sb) {
		printk("Unable obtain sb.\n");
		return -ECHILD;
	}

	/* allocate dentry parent */
	parent = d_alloc_anon(sb);
	if (!parent) {
		printk("Faild on d_alloc_anon()\n");
		return -ENOMEM;
	}

	/* allocate qstr name */
	string.len = strlen("BiscuitOS");
	string.name = kmemdup("BiscuitOS", string.len, GFP_KERNEL);
	dentry = d_alloc(parent, &string);
	if (!dentry) {
		printk("Unable alloc dentry.\n");
		return -ENOMEM;
	}

	printk("Dentry: %s\n", dentry->d_iname);

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
