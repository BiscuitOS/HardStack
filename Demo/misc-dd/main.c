/*
 * BiscuitOS MISC DD
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_SET		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

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

/* ioctl */
static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_SET:
		printk("IOCTL: BISCUITOS_SET.\n");
		break;
	case BISCUITOS_GET:
		printk("IOCTL: BISCUITOS_GET.\n");
		break;
	default:
		break;
	}
	return 0;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.open		= BiscuitOS_open,
	.release	= BiscuitOS_release,
	.write		= BiscuitOS_write,
	.read		= BiscuitOS_read,
	.unlocked_ioctl	= BiscuitOS_ioctl,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS MISC Device Driver");
