// SPDX-License-Identifier: GPL-2.0
/*
 * IOVEC: Copy Data from Userspace
 *
 * (C) 2023.03.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uio.h>

#define DEV_NAME		"BiscuitOS-IOV"
static char buffer[128];

static ssize_t BiscuitOS_write_iter(struct kiocb *kb, struct iov_iter *from)
{
	size_t len = iov_iter_count(from);

	if (copy_from_iter(buffer, len, from) != len) {
		printk("Sytem Error: copy_from_iter!\n");
		return -EINVAL;
	}

	printk("IOV-DATA: %s\n", buffer);

	return len;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.write_iter	= BiscuitOS_write_iter,
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
MODULE_DESCRIPTION("IOVEC");
