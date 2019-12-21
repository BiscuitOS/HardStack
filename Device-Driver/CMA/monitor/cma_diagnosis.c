/*
 * CMA Monitor
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

#include <linux/cma.h>
#include "cma.h"

extern struct cma cma_areas[MAX_CMA_AREAS];
static char buffer[128];

/* LDD Platform Name */
#define DEV_NAME "Diagnosis"

static void show_bitmap_long(unsigned long bitmap, int index)
{
	int nr = sizeof(unsigned long) * 8;
	int i;

	memset(buffer, 0x0, 128);
	for (i = 0; i < nr; i++) {
		if ((bitmap >> i) & 0x1)
			buffer[i] = 'X';
		else
			buffer[i] = 'Y';
	}
	buffer[i] = '\0';
	printk("[%05d]%s\n", index, buffer);
}

/* Read an bitmap from kernel to userspace
 *   On userspace:
 *   --> cat /sys/busy/platform/devices/Platform_attr.1/bitmap
 */
static ssize_t bitmap_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	struct cma *cma = &cma_areas[0];
	int bitmap_size = BITS_TO_LONGS(cma_bitmap_maxno(cma));
	int i;

	for (i = 0; i < bitmap_size; i++) {
		show_bitmap_long(cma->bitmap[i], i);
	}
	printk("Bitmap-length(unsigned long): %d-bits \n", bitmap_size);
	printk("CMA total: %ld-bits(1bits = 4K)\n", cma->count);
	printk("X-Used Y-Unused\n");
	return 0;
}

/* Write an bitmap from userspace to kernel
 *   On userspace:
 *   --> echo "0x28" > /sys/busy/platform/devices/Platform_attr.1/bitmap
 */
static ssize_t bitmap_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

/* bitmap attribute */
static struct device_attribute bitmap_attr =
                                __ATTR_RW(bitmap);

/* Probe: (LDD) Initialize Device */
static int Diagnosis_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &bitmap_attr);
	if (err) {
		dev_err(&pdev->dev, "Unable to create bitmap_attr");
		goto bitmap_err;
	}
	return 0;

bitmap_err:
	return err;
}

/* Remove: (LDD) Remove Device (Module) */
static int Diagnosis_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &bitmap_attr);
	return 0;
}

/* Platform Driver Information */
static struct platform_driver Diagnosis_driver = {
	.probe    = Diagnosis_probe,
	.remove   = Diagnosis_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static struct platform_device *pdev;
/* Module initialize entry */
static int __init Diagnosis_init(void)
{
	int ret;

	ret = platform_driver_register(&Diagnosis_driver);
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
static void __exit Diagnosis_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&Diagnosis_driver);
}

module_init(Diagnosis_init);
module_exit(Diagnosis_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CMA Diagnosis Device Driver");
