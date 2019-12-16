/*
 * Platfrom + Char
 *
 * (C) 2019.12.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        BiscuitOS_demo {
 *                compatible = "dma-buf, BiscuitOS";
 *                status = "okay";
 *        };
 * };
 *
 * On Core dtsi:
 *
 * include "DTS_demo.dtsi"
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>

/* LDD Platform Name */
#define DEV_NAME	"BiscuitOS_demo"

/* Char device */
static int demo_major;
static int demo_minor;
static struct class *demo_class;

struct demo_desc {
	struct device *device;
	struct cdev cdev;
	int index;
};

static const struct file_operations demo_fops;

static int demo_open(struct inode *inode, struct file *filp)
{
	struct demo_desc *desc;

	desc = container_of(inode->i_cdev, struct demo_desc, cdev);
	filp->private_data = desc;
	return 0;
}

static int demo_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

/* cdev interface create */
static int demo_cdev(struct demo_desc *desc)
{
	struct device *device;
	dev_t dev = 0;

	alloc_chrdev_region(&dev, demo_minor, 1, "BiscuitOS");
	demo_major = MAJOR(dev);
	cdev_init(&desc->cdev, &demo_fops);
	desc->cdev.owner = THIS_MODULE;
	cdev_add(&desc->cdev, dev, 1);

	/* class creete */
	demo_class = class_create(THIS_MODULE, "BiscuitOS_class");
	if (IS_ERR(demo_class)) {
		printk("BiscuitOS class not created\n");
		cdev_del(&desc->cdev);
		return PTR_ERR(demo_class);
	}

	/* device create */
	device = device_create(demo_class, NULL, dev, NULL, "BiscuitOS");
	if (IS_ERR(device)) {
		printk("BiscuitOS device not created\n");
		class_destroy(demo_class);
		cdev_del(&desc->cdev);
	}
	return 0;
}

/* Probe: (LDD) Initialize Device */
static int demo_probe(struct platform_device *pdev)
{
	struct demo_desc *desc;
	int err;

	desc = devm_kzalloc(&pdev->dev, sizeof(struct demo_desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	err = demo_cdev(desc);
	desc->index = 0x99;
	platform_set_drvdata(pdev, desc);

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int demo_remove(struct platform_device *pdev)
{
	struct demo_desc *desc;
	dev_t dev = MKDEV(demo_major, demo_minor);

	desc = platform_get_drvdata(pdev);
	device_destroy(demo_class, dev);
	cdev_del(&desc->cdev);
	kfree(desc);

	return 0;
}

static const struct file_operations demo_fops = {
	.owner		= THIS_MODULE,
	.open		= demo_open,
	.release	= demo_release,
};

static const struct of_device_id demo_of_match[] = {
	{ .compatible = "dma-buf, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, demo_of_match);

/* Platform Driver Information */
static struct platform_driver demo_driver = {
	.probe    = demo_probe,
	.remove   = demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= demo_of_match,
	},
};
module_platform_driver(demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS LDD");
