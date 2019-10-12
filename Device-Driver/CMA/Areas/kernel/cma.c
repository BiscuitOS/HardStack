/*
 * Contiguous Memory Allocate driver (LDD: CMA)
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cma.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/dma-contiguous.h>
#include <linux/platform_device.h>

/* mm/cma.h */
#include "cma.h"

/* LDD Driver Name */
#define DEV_NAME		"CMA_demo"
#define CMA_REGION_NUM		64
#define CMA_MEM_ALLOCATE	_IOW('m', 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW('m', 2, unsigned int)


/* CMA region information */
struct CMA_demo_info {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
};

/* CMA Memory Region */
struct CMA_demo_region {
	struct CMA_demo_info info;
	struct list_head list;
};

/* CMA manager information */
struct CMA_demo_manager {
	struct miscdevice misc;
	struct mutex lock;
	struct list_head head;
	struct device *dev;
};

/* Platform device */
static struct platform_device *pd;
struct CMA_demo_manager *manager;
extern struct cma *find_cma_by_name(const char *name);

static long CMA_demo_ioctl(struct file *filp, unsigned int cmd, 
							unsigned long arg)
{
	struct CMA_demo_region *region;
	struct CMA_demo_info info;
	unsigned int pool_size_order;
	unsigned long nr_pages;
	struct page *page;
	int found = 0;
	int rvl;

	switch (cmd) {
	case CMA_MEM_ALLOCATE:
		/* lock */
		mutex_lock(&manager->lock);
		/* Get information from userland */
		if (copy_from_user(&info, (void __user *)arg,
					sizeof(struct CMA_demo_info))) {
			printk(KERN_ERR "ALLOCATE: copy_from_user error\n");
			rvl = -EFAULT;
			goto err_user;
		}

		/* allocate new region */
		region = kzalloc(sizeof(*region), GFP_KERNEL);
		if (!region) {
			printk(KERN_ERR "ALLOCATE: no free memory.\n");
			rvl = -ENOMEM;
			goto err_alloc;
		}

		nr_pages = info.length >> PAGE_SHIFT;
		pool_size_order = get_order(info.length);
		/* Allocate memory from CMA */
		page = dma_alloc_from_contiguous(manager->dev, nr_pages,
				pool_size_order, GFP_KERNEL);
		if (!page) {
			printk(KERN_ERR "ALLOCATE: DMA allocate error\n");
			rvl = -ENOMEM;
			goto err_dma;
		}
		
		/* Insert region into manager */
		info.virt = (dma_addr_t)page_to_virt(page);
		info.phys = (dma_addr_t)page_to_phys(page);
		region->info.virt = info.virt;
		region->info.phys = info.phys;
		region->info.length = info.length;
		list_add(&region->list, &manager->head);

		/* export to userland */
		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			printk(KERN_ERR "ALLOCATE: copy_to_user error\n");
			rvl = -EINVAL;
			goto err_to;
		}
		/* unlock */
		mutex_unlock(&manager->lock);
		return 0;
	case CMA_MEM_RELEASE:
		mutex_lock(&manager->lock);
		if (copy_from_user(&info, (void __user *)arg,
							sizeof(info))) {
			printk(KERN_ERR "RELEASE: copy_from_user\n");
			rvl = -EINVAL;
			goto err_user;
		}
		/* Search region */
		list_for_each_entry(region, &manager->head, list) {
			if (region->info.phys == info.phys &&
				     region->info.length == info.length) {
				found = 1;
				break;
			}
		}
		if (!found) {
			printk(KERN_ERR "RELEASE: Can't find special region\n");
			rvl = -EINVAL;
			goto err_user;
		}
		/* Free contiguous memory */
		page = phys_to_page(info.phys);
		nr_pages = info.length >> PAGE_SHIFT;
		dma_release_from_contiguous(manager->dev, page, nr_pages);
		list_del(&region->list);
		kfree(region);
		mutex_unlock(&manager->lock);

		return 0;
	default:
		printk(KERN_INFO "CMA not support command.\n");
		return -EFAULT;
	}

err_to:
	list_del(&region->list);
	dma_release_from_contiguous(manager->dev, page, nr_pages);
err_dma:
	kfree(region);
err_alloc:

err_user:
	mutex_unlock(&manager->lock);
	return rvl;

}

static int CMA_demo_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long page;

	/* offset is physical address */
	page = offset >> PAGE_SHIFT;

	/* Remap */
	if (remap_pfn_range(vma, start, page, size, PAGE_SHARED)) {
		printk("REMAP: failed\n");
		return -EAGAIN;
	}

	vma->vm_flags &= ~VM_IO;
	vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);

	return 0;
}

/* file operations */
static struct file_operations CMA_demo_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= CMA_demo_ioctl,
	.mmap		= CMA_demo_mmap,
};

/* Module initialize entry */
static int __init CMA_demo_probe(struct platform_device *pdev)
{
	struct cma *cma;
	int rvl;

	/* Find special cma via name */
	cma = find_cma_by_name("BiscuitOS_cma");
	if (cma) {
		printk("Find CMA: %s\n", cma->name);
		pdev->dev.cma_area = cma;
	}

	/* CMA: Initialize device */
	manager = kzalloc(sizeof(struct CMA_demo_manager), GFP_KERNEL);
	if (!manager) {
		printk(KERN_ERR "Allocate memory failed\n");
		rvl = -ENOMEM;
		goto err_alloc;
	}

	/* Lock: initialize */
	mutex_init(&manager->lock);
	/* Misc: initialize */
	manager->misc.name  = DEV_NAME;
	manager->misc.minor = MISC_DYNAMIC_MINOR;
	manager->misc.fops  = &CMA_demo_fops;

	/* list: initialize */
	INIT_LIST_HEAD(&manager->head);

	/* Register Misc device */
	misc_register(&manager->misc);

	manager->dev = &pdev->dev;
	/* Set platform drvdata */
	platform_set_drvdata(pdev, manager);

	return 0;

err_alloc:
	return rvl;
}

/* Module exit entry */
static int __exit CMA_demo_remove(struct platform_device *pdev)
{
	struct CMA_demo_region *reg;

	/* Free all region */
	mutex_lock(&manager->lock);
	list_for_each_entry(reg, &manager->head, list)
		kfree(reg);
	mutex_unlock(&manager->lock);

	/* Un-Register Misc device */
	misc_deregister(&manager->misc);
	/* free memory */
	kfree(manager);
	manager = NULL;
	return 0;
}

/* Platform Driver Information */
static struct platform_driver CMA_demo_driver = {
	.probe    = CMA_demo_probe,
	.remove   = CMA_demo_remove,
	.driver = {
		.owner  = THIS_MODULE,
		.name   = DEV_NAME,
	},
};

/* Module initialize entry */
static int __init CMA_demo_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&CMA_demo_driver);
	if (ret) {
		printk("Unable register Platform driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	pd = platform_device_register_simple(DEV_NAME, -1, NULL, 0);
	if (IS_ERR(pd)) {
		printk("Unable register Platform device.\n");
		return -EBUSY;
	}

	return 0;
}

/* Module exit entry */
static void __exit CMA_demo_exit(void)
{
	platform_device_unregister(pd);
	platform_driver_unregister(&CMA_demo_driver);
}

module_init(CMA_demo_init);
module_exit(CMA_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Contiguous Memory Allocate (CMA) Device Driver");
