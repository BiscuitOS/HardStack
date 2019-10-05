/*
 * Character device driver
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
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>

/* DDL Platform Name */
#define DEV_NAME "Char_demo"
/* Character device Major number: Dynamic allocation */
#define CHAR_DEMO_MAJOR	0
/* Character device Minor number */
#define CHAR_DEMO_MINOR	2

/* Class model */
static struct class *Char_demo_class;
/* Device model */
static struct device *Char_demo_dev;
/* Dynamic major number */
static int Char_demo_major;

/* private data */
struct Char_demo_pdata {
	int num;
};

/* Open */
static int Char_demo_open(struct inode *inode, struct file *filp)
{
	struct Char_demo_pdata *pdata;

	/* allocate memory to pdata */
	pdata = kzalloc(sizeof(struct Char_demo_pdata), GFP_KERNEL);
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
static int Char_demo_release(struct inode *inode, struct file *filp)
{
	struct Char_demo_pdata *pdata = filp->private_data;

	/* Safe pointer */
	filp->private_data = NULL;

	/* free private data */
	kfree(pdata);
	return 0;
}

/* Read */
static ssize_t Char_demo_read(struct file *filp, char __user *buf,
			size_t len, loff_t *offset)
{
	struct Char_demo_pdata *pdata = filp->private_data;

	if (copy_to_user(buf, pdata, len)) {
		printk(KERN_ERR "Unable copy data to user.\n");
		return -EINVAL;
	}
	return len;
}

/* Write */
static ssize_t Char_demo_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	struct Char_demo_pdata *pdata = filp->private_data;

	if (copy_from_user(pdata, buf, len)) {
		printk(KERN_ERR "Unable copy data from user.\n");
		return -EINVAL;
	}
	printk("Data from userland: %d\n", pdata->num);

	return len;
}

/* file operations */
static struct file_operations Char_demo_fops = {
	.owner		= THIS_MODULE,
	.open		= Char_demo_open,
	.release	= Char_demo_release,
	.write		= Char_demo_write,
	.read		= Char_demo_read,
};

/* Module initialize entry */
static int __init Char_demo_init(void)
{
	int rvl;

	/* Register Character driver */
	rvl = register_chrdev(CHAR_DEMO_MAJOR, DEV_NAME, &Char_demo_fops);
	if (rvl < 0) {
		printk(KERN_ERR "Unable to register Character driver.\n");
		rvl = -ENODEV;
		goto err_chrdev;
	}
	Char_demo_major = rvl;

	/* Register into class */
	Char_demo_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(Char_demo_class)) {
		printk(KERN_ERR "Unable to register class subsystem.\n");
		rvl = PTR_ERR(Char_demo_class);
		goto err_class;
	}

	/* Register device into sysfs */
	Char_demo_dev = device_create(Char_demo_class, NULL, 
				MKDEV(Char_demo_major, CHAR_DEMO_MINOR), NULL, DEV_NAME);
	if (IS_ERR(Char_demo_dev)) {
		printk(KERN_ERR "Unable to register device subsystem.\n");
		rvl = PTR_ERR(Char_demo_dev);
		goto err_dev;
	}

	printk(KERN_INFO "%s - Major: %d Minor: %d\n", DEV_NAME, Char_demo_major,
									CHAR_DEMO_MINOR);
	return 0;

err_dev:
	class_destroy(Char_demo_class);
err_class:
	unregister_chrdev(Char_demo_major, DEV_NAME);
err_chrdev:

	return rvl;
}

/* Module exit entry */
static void __exit Char_demo_exit(void)
{
	device_destroy(Char_demo_class, MKDEV(CHAR_DEMO_MAJOR, CHAR_DEMO_MINOR));
	class_destroy(Char_demo_class);
	unregister_chrdev(Char_demo_major, DEV_NAME);
}

module_init(Char_demo_init);
module_exit(Char_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Character Device driver");
