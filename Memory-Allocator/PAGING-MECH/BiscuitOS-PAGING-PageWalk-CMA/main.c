// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with CMA
 *  - Enable Macro CONFIG_CMA=y
 *                 CONFIG_DMA_CMA=y
 *                 CONFIG_CMA_SIZE_MBYTES=32 
 *   
 * (C) 2023.08.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/dma-map-ops.h>
#include <linux/pagewalk.h>
#include <asm/pgtable.h>

#define DEV_NAME	"BiscuitOS-PageTable"
#define CMA_BUF_SIZE	0x1000000 /* 16MiB */
#define BISCUITOS_IO	0xBD
#define BS_WALK_PT	_IO(BISCUITOS_IO, 0x00)

static struct BiscuitOS_device {
	struct platform_device *pdev;
	/* DMA Memory Physical Address */
	dma_addr_t cma_addr;
	/* DMA Memory Virtual Address */
	char *cma_buffer;
} bdev;

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	if (pte_none(*pte))
		return 0;

	printk("VirtAddr: %#lx\n", addr);
	printk("PTEVal:   %#lx\n", pte_val(*pte));
	printk("PhysAddr: %#lx\n", pte_pfn(*pte) << PAGE_SHIFT);
	printk("CMA Phys: %#lx\n", (unsigned long)bdev.cma_addr);

	return 0;
}

static int BiscuitOS_test_walk(unsigned long addr, unsigned long next,
		struct mm_walk *walk) { return 0; /* Force Walk */ }

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pte_entry	= BiscuitOS_pte_entry,
	.test_walk	= BiscuitOS_test_walk,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return dma_common_mmap(&bdev.pdev->dev, vma, bdev.cma_buffer,
			bdev.cma_addr, vma->vm_end - vma->vm_start, 0);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct vm_area_struct *vma;

	mmap_write_lock_killable(current->mm);
	vma = find_vma(current->mm, arg);
	if (!vma) {
		mmap_write_unlock(current->mm);
		return -EINVAL;
	}

	switch (ioctl) {
	case BS_WALK_PT:
		walk_page_range(vma->vm_mm, arg, arg + PAGE_SIZE,
					&BiscuitOS_pwalk_ops, NULL);
		break;
	}
	mmap_write_unlock(current->mm);
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner          = THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.unlocked_ioctl	= BiscuitOS_ioctl,
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

	/* CMA Memory Allocate */
	bdev.cma_buffer = dma_alloc_coherent(&pdev->dev,
				CMA_BUF_SIZE, &bdev.cma_addr, GFP_KERNEL);
	if (!bdev.cma_buffer) {
		printk("System Error: DMA Memory Allocate.\n");
		return -ENOMEM;
	}
	/* MISC */
	return misc_register(&BiscuitOS_drv);
}

static int BiscuitOS_remove(struct platform_device *pdev)
{
	misc_deregister(&BiscuitOS_drv);
	dma_free_coherent(&pdev->dev, CMA_BUF_SIZE,
				bdev.cma_buffer, bdev.cma_addr);
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
