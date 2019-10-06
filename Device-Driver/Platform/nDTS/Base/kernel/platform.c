/*
 * Platform Bus (Normal without DTS)
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

/* DDL Platform Name */
#define DEV_NAME "Platform_demo"

/* Probe: (DDL) Initialize Device */
static int Platform_demo_probe(struct platform_device *pdev)
{
	/* Device Probe Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);

	return 0;
}

/* Remove: (DDL) Remove Device (Module) */
static int Platform_demo_remove(struct platform_device *pdev)
{
	/* Device Remove Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	
	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Platform_demo_shutdown(struct platform_device *pdev)
{
	/* Device Shutdown/Power-off Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Platform_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	/* Device Suspend/Sleep Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);

	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Platform_demo_resume(struct platform_device *pdev)
{
	/* Device Resume Procedure */
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);

	return 0;
}

/* Platform Device Release */
static void Platform_demo_dev_release(struct device *dev)
{
	dev->parent = NULL;
}

/* Platform Driver Information */
static struct platform_driver Platform_demo_driver = {
	.probe    = Platform_demo_probe,
	.remove   = Platform_demo_remove,
	.shutdown = Platform_demo_shutdown,
	.suspend  = Platform_demo_suspend,
	.resume   = Platform_demo_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Platform Device Information */
static struct platform_device Platform_demo_device = {
	.name = DEV_NAME,
	.id = 1,
	.dev = {
		.release = Platform_demo_dev_release,
	}
};

/* Module initialize entry */
static int __init Platform_demo_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&Platform_demo_driver);
	if (ret) {
		printk("Unable register Platform driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	ret = platform_device_register(&Platform_demo_device);
	if (ret) {
		printk("Unable register Platform device.\n");
		return -EBUSY;
	}

	return 0;
}

/* Module exit entry */
static void __exit Platform_demo_exit(void)
{
	platform_device_unregister(&Platform_demo_device);
	platform_driver_unregister(&Platform_demo_driver);
}

module_init(Platform_demo_init);
module_exit(Platform_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Device Driver without DTS");
