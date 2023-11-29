// SPDX-License-Identifier: GPL-2.0
/*
 * LAZYALLOC: DMA Memory
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

static int PFNMAP_pte(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = (struct vm_area_struct *)data;

	set_pte_at(vma->vm_mm, addr, pte, pte_mkspecial(
		pfn_pte(bdev.dma_addr >> PAGE_SHIFT, vma->vm_page_prot)));

	return 0;
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;

	vma->vm_flags |= VM_PFNMAP | VM_MIXEDMAP;
	apply_to_page_range(vma->vm_mm, address,
			PAGE_SIZE, PFNMAP_pte, vma);

	return VM_FAULT_NOPAGE;
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.fault	= vm_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return 0;
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

static void __exit BiscuitOS_exit(void)
{
	platform_device_unregister(bdev.pdev);
	platform_driver_unregister(&BiscuitOS_driver);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
