/*
 * UIO Memory Device Driver
 *
 * (C) 2019.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/uio_driver.h>

#define DEV_NAME	"UIO_demo"
#define UIO_VERSION	"0.11"

struct uio_info uinfo = {
	.name = DEV_NAME,
	.version = UIO_VERSION,
	.irq = UIO_IRQ_NONE,
};

static int UIO_demo_probe(struct platform_device *pdev)
{
	int ret;

	uinfo.mem[0].addr = (unsigned long)kzalloc(1024, GFP_KERNEL);
	if (uinfo.mem[0].addr == 0) {
		printk("System no free meory.\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	uinfo.mem[0].memtype = UIO_MEM_LOGICAL;
	uinfo.mem[0].size = 1024;

	/* Register UIO device */
	ret = uio_register_device(&pdev->dev, &uinfo);
	if (ret) {
		printk("Register UIO error\n");
		ret = -ENODEV;
		goto err_uio;
	}

	platform_set_drvdata(pdev, &uinfo);

	printk("UIO -> %#lx (length: %ld)\n", 
			(unsigned long)uinfo.mem[0].addr,
			(unsigned long)uinfo.mem[0].size);
	return 0;

err_uio:
	kfree((void *)(unsigned long)uinfo.mem[0].addr);
err_alloc:
	return ret;
}

static int UIO_demo_remove(struct platform_device *pdev)
{
	struct uio_info *uinfo = platform_get_drvdata(pdev);

	uio_unregister_device(uinfo);

	kfree((void *)(unsigned long)uinfo->mem[0].addr);

	return 0;
}

static struct platform_driver UIO_demo_drv = {
	.probe = UIO_demo_probe,
	.remove = UIO_demo_remove,
	.driver = {
		.name = DEV_NAME,
		.owner = THIS_MODULE,
	},
};

static struct platform_device *UIO_demo_dev;
static int __init UIO_demo_init(void)
{
	UIO_demo_dev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);

	return platform_driver_register(&UIO_demo_drv);
}

static void __exit UIO_demo_exit(void)
{
	platform_device_unregister(UIO_demo_dev);

	platform_driver_unregister(&UIO_demo_drv);
}

module_init(UIO_demo_init);
module_exit(UIO_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BuddyZhang1 BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("UIO Memory Device Driver Module");
