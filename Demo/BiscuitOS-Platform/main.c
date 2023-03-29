// SPDX-License-Identifier: GPL-2.0
/*
 * Simple Platform Device Driver
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

/* LDD Platform Name */
#define DEV_NAME	"BiscuitOS"

static struct platform_device *pdev;

/* Probe: (LDD) Initialize Device */
static int BiscuitOS_probe(struct platform_device *pdev)
{
	/* Device Probe Procedure */
	printk("BiscuitOS Porbeing...\n");

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int BiscuitOS_remove(struct platform_device *pdev)
{
	/* Device Remove Procedure */
	printk("BiscuitOS Removing...\n");
	
	return 0;
}

/* Platform Driver Information */
static struct platform_driver BiscuitOS_driver = {
	.probe    = BiscuitOS_probe,
	.remove   = BiscuitOS_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	int ret;

	ret = platform_driver_register(&BiscuitOS_driver);
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
static void __exit BiscuitOS_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&BiscuitOS_driver);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Simple Platform Device Driver");
