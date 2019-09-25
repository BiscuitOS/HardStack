/*
 * Platform Bus (DTS DDL)
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Private DTS file: DTS_demo.dtsi
 *
 * / {
 *        Platform_demo {
 *                compatible = "Platform_demo, BiscuitOS";
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

static const struct of_device_id Platform_demo_of_match[] = {
	{ .compatible = "Platform_demo, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, Platform_demo_of_match);

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
		.of_match_table	= Platform_demo_of_match,
	},
};
module_platform_driver(Platform_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Device Driver with DTS");
