// SPDX-License-Identifier: GPL-2.0
/*
 * CACE SCENE: copy_{from}_user
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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
static char buffer[128];

static ssize_t BiscuitOS_write(struct file *filp,
		const char __user *buf, size_t len, loff_t *offset)
{
	if (copy_from_user(buffer, buf, len) != 0) {
		printk(KERN_ERR "Unable copy data to user.\n");
		return -EINVAL;
	}

	printk("USER-DATA: %s\n", buffer);

	return len;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.write		= BiscuitOS_write,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);
	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("COPY USER NOCACHE");
