/*
 * Simple Platform Device Driver
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
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/device.h>

#define DEV_NAME	"Demo"

struct Demo_node {
	struct Demo_node *parent;
	unsigned long index;
};

static void devm_Demo_release(struct device *dev, void *res)
{
	printk("Release Device resource.\n");
}

/* Probe: (LDD) Initialize Device */
static int Demo_probe(struct platform_device *pdev)
{
	struct Demo_node **ptr, *p;

	ptr = devres_alloc(devm_Demo_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr) {
		return -ENOMEM;
	}

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!IS_ERR(p)) {
		*ptr = p;
	} else {
		devres_free(ptr);
	}
	platform_set_drvdata(pdev, ptr);
	printk("Allocate Device Resource.\n");

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int Demo_remove(struct platform_device *pdev)
{
	struct Demo_node **ptr = platform_get_drvdata(pdev);
	struct Demo_node *p = *ptr;
	
	kfree(p);
	devres_free(ptr);

	return 0;
}

/* Platform Driver Information */
static struct platform_driver Demo_driver = {
	.probe    = Demo_probe,
	.remove   = Demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static struct platform_device *pdev;
/* Module initialize entry */
static int __init Demo_init(void)
{
	int ret;

	ret = platform_driver_register(&Demo_driver);
	if (ret) {
		printk("Error: Platform driver register.\n");
		return -EBUSY;
	}

	pdev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);
	if (IS_ERR(pdev)) {
		printk("Error: Platform device register\n");
		return PTR_ERR(pdev);
	}
	return 0;
}

/* Module exit entry */
static void __exit Demo_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&Demo_driver);
}

module_init(Demo_init);
module_exit(Demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Simple Platform Device Driver");
