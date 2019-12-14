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
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mount.h>

/* LDD Platform Name */
#define DEV_NAME	"Demo"

/* Write */
static ssize_t Demo_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	struct path path;
	struct vfsmount *mount;
	struct dentry *dentry;
	int ret;

	/* ... and get the mountpoint */
	ret = user_path(buf, &path);
	if (ret) {
		printk("Error: Unable to get mountpoint.\n");
		return -EINVAL;
	}

	mount = path.mnt;
	dentry = path.dentry;

	/* Mount Pointer */
	printk("Mount Filesytem: %s\n", mount->mnt_sb->s_type->name);
	printk("Mount Pointer:   %s\n", mount->mnt_root->d_iname);
	printk("Current Dirent:  %s\n", dentry->d_iname);

	return len;
}

/* file operations */
static struct file_operations Demo_fops = {
	.owner		= THIS_MODULE,
	.write		= Demo_write,
};

/* Misc device driver */
static struct miscdevice Demo_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &Demo_fops,
};

/* Module initialize entry */
static int __init Demo_init(void)
{
	/* Register Misc device */
	misc_register(&Demo_drv);
	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&Demo_drv);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS");
