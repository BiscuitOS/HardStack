/*
 * Misc device
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
#include <linux/slab.h>
#include <linux/uaccess.h>

/* DDL Platform Name */
#define DEV_NAME "Misc_demo"

/* private data */
struct Misc_demo_pdata {
	int num;
};

/* Open */
static int Misc_demo_open(struct inode *inode, struct file *filp)
{
	struct Misc_demo_pdata *pdata;

	/* allocate memory to pdata */
	pdata = kzalloc(sizeof(struct Misc_demo_pdata), GFP_KERNEL);
	if (!pdata) {
		printk(KERN_ERR "No free memory!\n");
		return -ENOMEM;
	}
	pdata->num = 0x91;

	/* Store on file */
	filp->private_data = pdata;

	return 0;
}

/* Release */
static int Misc_demo_release(struct inode *inode, struct file *filp)
{
	struct Misc_demo_pdata *pdata = filp->private_data;

	/* Safe pointer */
	filp->private_data = NULL;

	/* free private data */
	kfree(pdata);
	return 0;
}

/* Read */
static ssize_t Misc_demo_read(struct file *filp, char __user *buf,
			size_t len, loff_t *offset)
{
	struct Misc_demo_pdata *pdata = filp->private_data;

	if (copy_to_user(buf, pdata, len)) {
		printk(KERN_ERR "Unable copy data to user.\n");
		return -EINVAL;
	}
	return len;
}

/* Write */
static ssize_t Misc_demo_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	struct Misc_demo_pdata *pdata = filp->private_data;

	if (copy_from_user(pdata, buf, len)) {
		printk(KERN_ERR "Unable copy data from user.\n");
		return -EINVAL;
	}
	printk("Data from userland: %d\n", pdata->num);

	return len;
}

/* file operations */
static struct file_operations Misc_demo_fops = {
	.owner		= THIS_MODULE,
	.open		= Misc_demo_open,
	.release	= Misc_demo_release,
	.write		= Misc_demo_write,
	.read		= Misc_demo_read,
};

/* Misc device driver */
static struct miscdevice Misc_demo_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &Misc_demo_fops,
};

/* Module initialize entry */
static int __init Misc_demo_init(void)
{
	/* Register Misc device */
	misc_register(&Misc_demo_drv);
	return 0;
}

/* Module exit entry */
static void __exit Misc_demo_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&Misc_demo_drv);
}

module_init(Misc_demo_init);
module_exit(Misc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Device Driver with DTS");
