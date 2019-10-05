/*
 * Platform Bus (store/show)
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
#define DEV_NAME "Platform_attr"

static unsigned long Hexadecimal;
static unsigned long Integer;
static char String[256];

/* Read an Hexadecimal from kernel to userspace
 *   On userspace:
 *   --> cat /sys/busy/platform/devices/Platform_attr.1/Hexadecimal
 */
static ssize_t Hexadecimal_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%lx", Hexadecimal);
}

/* Write an Hexadecimal from userspace to kernel
 *   On userspace:
 *   --> echo "0x28" > /sys/busy/platform/devices/Platform_attr.1/Hexadecimal
 */
static ssize_t Hexadecimal_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	sscanf(buf, "%lx", &Hexadecimal);
	return size;
}

/* Read an Integer from kernel to userspace
 *   On userspace:
 *   --> cat /sys/busy/platform/devices/Platform_attr.1/Integer
 */
static ssize_t Integer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%ld", Integer);
}

/* Write an Integer from userspace to kernel
 *   On userspace
 *   --> echo 2019 > /sys/busy/platform/devices/Platform_attr.1/Integer
 */
static ssize_t Integer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	sscanf(buf, "%ld", &Integer);
	return size;
}

/* Read an String from kernel to userspace
 *   On userspace
 *   --> cat /sys/busy/platform/devices/Platform_attr.1/String
 */
static ssize_t String_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s", String);
}

/* Write an string from userspace to kernel
 *   On usersapce
 *   --> echo "BiscuitOS" > /sys/busy/platform/devices/Platform_attr.1/String
 */
static ssize_t String_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	sscanf(buf, "%s", String);
	return size;
}

/* Hexadecimal attribute */
static struct device_attribute Hexadecimal_attr =
				__ATTR_RW(Hexadecimal);
/* Integer attribute */
static struct device_attribute Integer_attr = 
				__ATTR_RW(Integer);
/* String attribute */
static struct device_attribute String_attr = 
				__ATTR_RW(String);

/* Probe: (DDL) Initialize Device */
static int Platform_demo_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &Hexadecimal_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create Hexadecimal_attr");
		goto Hexadecimal_err;
	}
	err = device_create_file(&pdev->dev, &Integer_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create Integer_attr");
		goto Integer_err;
	}
	err = device_create_file(&pdev->dev, &String_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create String_attr");
		goto String_err;
	}

	return 0;

String_err:
	device_remove_file(&pdev->dev, &Integer_attr);
Integer_err:
	device_remove_file(&pdev->dev, &Hexadecimal_attr);
Hexadecimal_err:
	return err;
}

/* Remove: (DDL) Remove Device (Module) */
static int Platform_demo_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &String_attr);
	device_remove_file(&pdev->dev, &Integer_attr);
	device_remove_file(&pdev->dev, &Hexadecimal_attr);

	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Platform_demo_shutdown(struct platform_device *pdev)
{
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Platform_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Platform_demo_resume(struct platform_device *pdev)
{
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
MODULE_DESCRIPTION("Platform Device Driver Attribute");
