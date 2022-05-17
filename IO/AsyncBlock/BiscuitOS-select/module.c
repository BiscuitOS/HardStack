/*
 * BiscuitOS EPOLL/POLL/SELECT Driver
 *
 * (C) 2022.05.16 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/poll.h>

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS"
#define BISCUITOS_TIMER_PERIOD	2000 /* 2s */

/* Poll */
static wait_queue_head_t BiscuitOS_wait;
static struct timer_list BiscuitOS_timer;
static int poll_size;

/* Emulate data ready */
static void BiscuitOS_timer_handler(struct timer_list *unused)
{
	/* Emulate data recv */
	poll_size = 20;
	wake_up_interruptible(&BiscuitOS_wait);

	mod_timer(&BiscuitOS_timer,
		jiffies + msecs_to_jiffies(BISCUITOS_TIMER_PERIOD));
}

/* Read */
static ssize_t BiscuitOS_read(struct file *filp, char __user *buf,
			size_t len, loff_t *offset)
{
	/* Emulate read operation */
	poll_size = 0;
	return len;
}

/* poll */
static 
__poll_t BiscuitOS_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned long mask = 0;

	poll_wait(filp, &BiscuitOS_wait, wait);
	if (poll_size)
		mask = POLLIN;
	else
		mask = 0;

	return mask;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.read		= BiscuitOS_read,
	.poll		= BiscuitOS_poll,
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
	init_waitqueue_head(&BiscuitOS_wait);
	timer_setup(&BiscuitOS_timer, BiscuitOS_timer_handler, 0);
	mod_timer(&BiscuitOS_timer,
			jiffies + msecs_to_jiffies(BISCUITOS_TIMER_PERIOD));
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
	del_timer(&BiscuitOS_timer);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS EPOLL/POLL/SELECT Device Driver");
