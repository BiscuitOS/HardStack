/*
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

/* LDD Platform Name */
#define DEV_NAME	"Demo"
#define CACHE_SIZE	0x30
/* SLAB cache */
static struct kmem_cache *BiscuitOS_cachep __read_mostly;

/* Write */
static ssize_t Demo_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *offset)
{
	char *name;
	int ret;

	/* allocate memory from CACHE */
	name = kmem_cache_alloc(BiscuitOS_cachep, GFP_KERNEL);
	if (unlikely(!name))
		return -ENOMEM;

	/* copy context from userspace and store on cache */
	ret = strncpy_from_user(name, buf, CACHE_SIZE);
	if (unlikely(ret < 0)) {
		kmem_cache_free(BiscuitOS_cachep, name);
		return ret;
	}
	printk("NAME: %s\n", name);

	/* Release cache */
	kmem_cache_free(BiscuitOS_cachep, name);
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
	
	BiscuitOS_cachep = kmem_cache_create_usercopy("BiscuitOS cache",
				CACHE_SIZE,
				0,
				SLAB_HWCACHE_ALIGN | SLAB_PANIC,
				0,
				CACHE_SIZE,
				NULL);
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
MODULE_DESCRIPTION("Device Driver");
