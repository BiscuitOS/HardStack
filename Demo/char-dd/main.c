/*
 * BiscuitOS Char DD Demo
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>

/* Char DD Name */
#define DEV_NAME		"BiscuitOS"
/* Character device Major number: Dynamic allocation */
#define BISCUITOS_MAJOR		0
/* Character device Minor number */
#define BISCUITOS_MINOR		2

/* Class model */
static struct class *BiscuitOS_class;
/* Device model */
static struct device *BiscuitOS_dev;
/* Dynamic major number */
static int BiscuitOS_major;

/* private data */
struct BiscuitOS_pdata {
	int num;
};

/* Open */
static int BiscuitOS_open(struct inode *inode, struct file *filp)
{
	struct BiscuitOS_pdata *pdata;

	/* allocate memory to pdata */
	pdata = kzalloc(sizeof(struct BiscuitOS_pdata), GFP_KERNEL);
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
static int BiscuitOS_release(struct inode *inode, struct file *filp)
{
	struct BiscuitOS_pdata *pdata = filp->private_data;

	/* Safe pointer */
	filp->private_data = NULL;

	/* free private data */
	kfree(pdata);
	return 0;
}

/* Read */
static ssize_t BiscuitOS_read(struct file *filp, char __user *buf,
			size_t len, loff_t *offset)
{
	struct BiscuitOS_pdata *pdata = filp->private_data;

	if (copy_to_user(buf, pdata, len)) {
		printk(KERN_ERR "Unable copy data to user.\n");
		return -EINVAL;
	}
	return len;
}

/* Write */
static ssize_t BiscuitOS_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	struct BiscuitOS_pdata *pdata = filp->private_data;

	if (copy_from_user(pdata, buf, len)) {
		printk(KERN_ERR "Unable copy data from user.\n");
		return -EINVAL;
	}
	printk("Data from userland: %d\n", pdata->num);

	return len;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.open		= BiscuitOS_open,
	.release	= BiscuitOS_release,
	.write		= BiscuitOS_write,
	.read		= BiscuitOS_read,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int rvl;

	/* Register Character driver */
	rvl = register_chrdev(BISCUITOS_MAJOR, DEV_NAME, &BiscuitOS_fops);
	if (rvl < 0) {
		printk(KERN_ERR "Unable to register Character driver.\n");
		rvl = -ENODEV;
		goto err_chrdev;
	}
	BiscuitOS_major = rvl;

	/* Register into class */
	BiscuitOS_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(BiscuitOS_class)) {
		printk(KERN_ERR "Unable to register class subsystem.\n");
		rvl = PTR_ERR(BiscuitOS_class);
		goto err_class;
	}

	/* Register device into sysfs */
	BiscuitOS_dev = device_create(BiscuitOS_class, NULL, 
				MKDEV(BiscuitOS_major, BISCUITOS_MINOR), 
					NULL, DEV_NAME);
	if (IS_ERR(BiscuitOS_dev)) {
		printk(KERN_ERR "Unable to register device subsystem.\n");
		rvl = PTR_ERR(BiscuitOS_dev);
		goto err_dev;
	}

	printk(KERN_INFO "%s - Major: %d Minor: %d\n", DEV_NAME, 
					BiscuitOS_major, BISCUITOS_MINOR);
	return 0;

err_dev:
	class_destroy(BiscuitOS_class);
err_class:
	unregister_chrdev(BiscuitOS_major, DEV_NAME);
err_chrdev:

	return rvl;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	device_destroy(BiscuitOS_class, 
			MKDEV(BISCUITOS_MAJOR, BISCUITOS_MINOR));
	class_destroy(BiscuitOS_class);
	unregister_chrdev(BiscuitOS_major, DEV_NAME);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Character Device driver");
