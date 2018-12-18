/*
 * Copyright (C) 2018.12.18 buddy.zhang@aliyun.com
 *
 * Misc device driver demo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define DEV_NAME "misc_demo"

/* Open entence on driver */
static int misc_demo_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/* Release entence on driver */
static int misc_demo_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* Read entence on driver */
static ssize_t misc_demo_read(struct file *filp, char __user *buffer,
                         size_t count, loff_t *offset)
{
    return 0;
}

/* Write entence on driver */
static ssize_t misc_demo_write(struct file *filp, const char __user *buf,
                               size_t count, loff_t *offset)
{
    return 0;
}

/* file_operations structure */
static struct file_operations misc_demo_fops = {
    .owner     = THIS_MODULE,
    .open      = misc_demo_open,
    .release   = misc_demo_release,
    .write     = misc_demo_write,
    .read      = misc_demo_read,
};

/* Misc device entence */
static struct miscdevice misc_demo_misc = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name     = DEV_NAME,
    .fops     = &misc_demo_fops,
};

/* driver inited entence */
static __init int misc_demo_init(void)
{
    misc_register(&misc_demo_misc);
    return 0;
}

/* driver exit entence */
static __exit void misc_demo_exit(void)
{
    misc_deregister(&misc_demo_misc);
}
module_init(misc_demo_init);
module_exit(misc_demo_exit);

/* Module information */
MODULE_LICENSE("GPL v2");
