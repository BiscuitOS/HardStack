/*
 * .pushsection/.popsection
 *
 * (C) 2020.01.08 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#define DEV_NAME "pushsection"

static inline unsigned long load_unaligned(unsigned long *addr)
{
	unsigned long ret;

	asm volatile (
	"1:	ldr	%0, [%1]			\n"
	"2:						\n"
	"	.pushsection .text.fixup,\"ax\"		\n"
	"	.align 2				\n"
	"3:	mov	%0, #0x89			\n"
	"	b	2b				\n"
	"	.popsection				\n"
	"	.pushsection __ex_table,\"ax\"		\n"
	"	.align	3				\n"
	"	.long	1b, 3b				\n"
	"	.popsection				\n"
	: "=&r" (ret)
	: "r" (addr));

	return ret;
}

/* Trigger a unliagned exception
 *   On userspace:
 *   --> cat /sys/bus/platform/devices/pushsection.1/unaligned
 */
static ssize_t unaligned_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long *addr;
	unsigned long value;
	
	addr = kzalloc(sizeof(unsigned long), GFP_KERNEL);
	*addr = 0x8976;

	/* Pass a unliagned address */
	value = load_unaligned((unsigned long *)((unsigned long)addr + 1));
	printk("Value: %#lx\n", value);
	kfree(addr);

	return 0;
}

/* Normal aligned show
 *   On userspace:
 *   --> cat /sys/bus/platform/device/pushsection.1/aligned
 */
static ssize_t aligned_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned long *addr;
	unsigned long value;

	addr = kzalloc(sizeof(unsigned long), GFP_KERNEL);
	*addr = 0x8976;

	/* Pass a aligned address */
	value = load_unaligned(addr);
	printk("Value: %#lx\n", value);
	kfree(addr);

	return 0;
}

/* Unaligned attribute */
static struct device_attribute unaligned_attr =
				__ATTR_RO(unaligned);
/* aligned attribute */
static struct device_attribute aligned_attr = 
				__ATTR_RO(aligned);

/* Probe: (LDD) Initialize Device */
static int pushsection_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &unaligned_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create unaligned_attr");
		goto unaligned_err;
	}
	err = device_create_file(&pdev->dev, &aligned_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create aligned_attr");
		goto aligned_err;
	}
	return 0;

aligned_err:
	device_remove_file(&pdev->dev, &unaligned_attr);
unaligned_err:
	return err;
}

/* Remove: (LDD) Remove Device (Module) */
static int pushsection_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &aligned_attr);
	device_remove_file(&pdev->dev, &unaligned_attr);

	return 0;
}

/* Platform Driver Information */
static struct platform_driver pushsection_driver = {
	.probe    = pushsection_probe,
	.remove   = pushsection_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static struct platform_device *pdev;
/* Module initialize entry */
static int __init pushsection_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&pushsection_driver);
	if (ret) {
		printk("Unable register Platform driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	pdev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);
	if (IS_ERR(pdev)) {
		printk("Error: Platform device register\n");
		return PTR_ERR(pdev);
	}

	return 0;
}

/* Module exit entry */
static void __exit pushsection_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&pushsection_driver);
}

module_init(pushsection_init);
module_exit(pushsection_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION(".pushsection/.popsection DD");
