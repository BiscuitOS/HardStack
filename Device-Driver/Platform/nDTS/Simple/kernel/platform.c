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

/* LDD Platform Name */
#define DEV_NAME "Platform_demo"

/* Probe: (LDD) Initialize Device */
static int Platform_demo_probe(struct platform_device *pdev)
{
	/* Device Probe Procedure */
	printk("Probe\n");

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int Platform_demo_remove(struct platform_device *pdev)
{
	/* Device Remove Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	
	return 0;
}

/* Platform Driver Information */
static struct platform_driver Platform_demo_driver = {
	.probe    = Platform_demo_probe,
	.remove   = Platform_demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static struct platform_device *pdev;
/* Module initialize entry */
static int __init Platform_demo_init(void)
{
	int ret;

	ret = platform_driver_register(&Platform_demo_driver);
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
static void __exit Platform_demo_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&Platform_demo_driver);
}

module_init(Platform_demo_init);
module_exit(Platform_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Simple Platform Device Driver");
