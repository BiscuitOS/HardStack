/*
 * Anonymous file on BiscuitOS
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
#include <linux/anon_inodes.h>
#include <linux/errno.h>
#include <linux/file.h>

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_SET		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

/* File operation for anonymous file */
static struct file_operations BiscuitOS_anonymous_fops;

/* Create anonymous file */
static int BiscuitOS_anonymous_file(void)
{
	struct file *file;
	int fd;

	/* Get an unused fd */
	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		printk("ERROR[%d]: Get unused fd failed.\n", fd);
		return fd;
	}

	/* Create anonymous file */
	file = anon_inode_getfile("BiscuitOS-anonymous", 
				&BiscuitOS_anonymous_fops, NULL, O_RDWR);
	if (IS_ERR(file)) {
		printk("ERROR[%ld]: Create anonymous file failed.\n",
						PTR_ERR(file));
		put_unused_fd(fd);
		return PTR_ERR(file);
	}

	/* Bind fd and file */
	fd_install(fd, file);
	return 0;
}

/* ioctl */
static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_SET:
		return BiscuitOS_anonymous_file();
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
MODULE_DESCRIPTION("BiscuitOS Anonymous file");
