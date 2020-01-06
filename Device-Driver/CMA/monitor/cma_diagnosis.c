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
static int cma_index = 0;

/* LDD Platform Name */
#define DEV_NAME 		"Diagnosis"
#define CMA_DRAP_LEN		16
#define STACK_BUFFER_SIZE	256

/* Draw bitmap */
static void draw_bitmap(unsigned long *bitmap, int width, int index)
{
	int bytes = sizeof(unsigned long);
        char buffer_show[STACK_BUFFER_SIZE];
	char *begin;
        int i;

        for (i = 0; i < width; i++) {
                unsigned long bitmap_val = bitmap[i];
		begin = buffer_show + i * bytes * 2;

		if (bytes == 8) {
			sprintf(begin, "%.16lx", bitmap_val);
		} else {
			sprintf(begin, "%.8lx", bitmap_val);
		}
        }
	begin = buffer_show + i * bytes * 2;
	begin[0] = '\0';
	printk("[%06d] [%s]\n", index / width, buffer_show);
}

/* Read an bitmap from kernel to userspace
 *   On userspace:
 *   --> cat /sys/busy/platform/devices/Platform_attr.1/bitmap
 */
static ssize_t bitmap_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	struct cma *cma = &cma_areas[cma_index];
	int bitmap_size = BITS_TO_LONGS(cma_bitmap_maxno(cma));
	int idx;

	printk("\nCMA area: %s\n\n", cma->name);
	for (idx = 0; idx < bitmap_size; idx += CMA_DRAP_LEN)
		draw_bitmap(cma->bitmap + idx, CMA_DRAP_LEN, idx);
	printk("\nCMA total size: %#lx\n", cma->count * 4096);

	return 0;
}

/* Write an bitmap from userspace to kernel
 *   On userspace:
 *   --> echo "0" > /sys/busy/platform/devices/Platform_attr.1/bitmap
 */
static ssize_t bitmap_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size)
{
	sscanf(buf, "%d", &cma_index);
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
