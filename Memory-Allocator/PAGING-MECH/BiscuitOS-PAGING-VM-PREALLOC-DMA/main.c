// SPDX-License-Identifier: GPL-2.0
/*
 * PREALLOC: DMA-MAPPING Memory
 * 
 * (C) 2023.11.19 BiscuitOS <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-map-ops.h>

#define DEV_NAME	"BiscuitOS-DMA"
#define DMA_BUF_SIZE	0x100000

static struct BiscuitOS_device {
	struct platform_device *pdev;
	/* DMA Memory Physical Address */
	dma_addr_t dma_addr;
	/* DMA Memory Virtual Address */
	char *dma_buffer;
} bdev;

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return dma_common_mmap(&bdev.pdev->dev, vma, bdev.dma_buffer,
			bdev.dma_addr, vma->vm_end - vma->vm_start, 0);
}

static struct file_operations BiscuitOS_fops = {
	.owner          = THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = DEV_NAME,
	.fops   = &BiscuitOS_fops,
};

static int BiscuitOS_probe(struct platform_device *pdev)
{
	/* Force */
	pdev->dev.dma_ops = NULL;

	/* DMA Memory Allocate */
	bdev.dma_buffer = dma_alloc_coherent(&pdev->dev,
				DMA_BUF_SIZE, &bdev.dma_addr, GFP_KERNEL);
	if (!bdev.dma_buffer) {
		printk("System Error: DMA Memory Allocate.\n");
		return -ENOMEM;
	}

	/* MISC */
	return misc_register(&BiscuitOS_drv);
}

static int BiscuitOS_remove(struct platform_device *pdev)
{
	misc_deregister(&BiscuitOS_drv);
	dma_free_coherent(&pdev->dev, DMA_BUF_SIZE,
				bdev.dma_buffer, bdev.dma_addr);
	return 0;
}

static struct platform_driver BiscuitOS_driver = {
	.probe    = BiscuitOS_probe,
	.remove   = BiscuitOS_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

static int __init BiscuitOS_init(void)
{
	int ret;

	ret = platform_driver_register(&BiscuitOS_driver);
	if (ret) {
		printk("Error: Platform driver register.\n");
		return -EBUSY;
	}

	bdev.pdev = platform_device_register_simple(DEV_NAME, 1, NULL, 0);
	if (IS_ERR(bdev.pdev)) {
		printk("Error: Platform device register\n");
		return PTR_ERR(bdev.pdev);
	}
	return 0;
}
device_initcall(BiscuitOS_init);
