/*
 * Contiguous Memory Allocate Debris Issue
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
#include <linux/of_platform.h>

/* mm/cma.h */
#include "cma.h"

/* LDD Driver Name */
#define DEV_NAME		"BiscuitOS_CMA"
#define CMA_REGION_NUM		64
#define CMA_DRAP_LEN		2 /* Draw width in bytes */
#define CMA_MEM_ALLOCATE	_IOW('m', 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW('m', 2, unsigned int)

/* CMA region information */
struct CMA_info {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
};

/* CMA Memory Region */
struct CMA_region {
	struct CMA_info info;
	struct list_head list;
};

/* CMA manager information */
struct CMA_manager {
	struct miscdevice misc;
	struct mutex lock;
	struct list_head head;
	struct device *dev;
};

/* Platform device */
static struct platform_device *pd;
struct CMA_manager *manager;
extern struct cma *find_cma_by_name(const char *name);
static struct file_operations CMA_fops;
static const char *cma_name;

static long CMA_ioctl(struct file *filp, unsigned int cmd, 
							unsigned long arg)
{
	struct CMA_region *region;
	struct CMA_info info;
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
					sizeof(struct CMA_info))) {
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

static int CMA_mmap(struct file *filp, struct vm_area_struct *vma)
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

/* Draw bitmap */
static void draw_bitmap(unsigned long *bitmap, int width, int index)
{
	int len = sizeof(unsigned long) * 8; /* bits */
	char buffer_show[128];
	int i, j;

	for (i = 0; i < width; i++) {
		unsigned long bitmap_val = bitmap[i];

		for (j = 0; j < len; j++)
			if ((bitmap_val >> j) & 0x1)
				buffer_show[i * len + j] = 'X'; /* Used */
			else
				buffer_show[i * len + j] = '.'; /* Free */
	}
	buffer_show[width * len] = '\0';
	printk("[%04d] [%s]\n", index, buffer_show);
}

/* Read an CMA area bitmap
 *   On userspace:
 *   --> cat /sys/bus/platform/devices/BiscuitOS_CMA/bitmap
 */
static ssize_t bitmap_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct cma *cma = dev->cma_area;
	int bitmap_size = BITS_TO_LONGS(cma_bitmap_maxno(cma));
	int idx;

	printk("\nCMA area: %s\n\n", cma->name);
	for (idx = 0; idx < bitmap_size; idx += CMA_DRAP_LEN)
		draw_bitmap(cma->bitmap + idx, CMA_DRAP_LEN, idx);
	printk("\nCMA total size: %#lx\n", cma->count * 4096);
	printk("CMA: X - used . - free (1bit = 4KB)\n");
	return 0;
	
}

/* Setup CMA area
 *   On userspace:
 *   --> echo "BiscuitOS_cma" > /sys/bus/platform/devices/BiscuitOS_CMA/bitmap
 */
static ssize_t bitmap_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return size;
}

/* bitmap attribute */
static struct device_attribute bitmap_attr = __ATTR_RW(bitmap);

/* Module initialize entry */
static int __init CMA_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct cma *cma;
	int rvl;

	/* Find CMA area name on current DT */
	rvl = of_property_read_string(np, "CMA-name", &cma_name);
	if (rvl >= 0) {
		/* Find special cma via name */
		cma = find_cma_by_name(cma_name);
		if (cma) {
			printk("Configure CMA area as: %s\n", cma->name);
			pdev->dev.cma_area = cma;
		}
	}

	/* CMA: Initialize device */
	manager = kzalloc(sizeof(struct CMA_manager), GFP_KERNEL);
	if (!manager) {
		printk(KERN_ERR "Allocate memory failed\n");
		rvl = -ENOMEM;
		goto err_alloc;
	}

	/* /sys interface */
	rvl = device_create_file(&pdev->dev, &bitmap_attr);
	if (rvl) {
		dev_err(&pdev->dev, "Unable to create bitmap attribute\n");
		goto err_file;
	}

	/* Lock: initialize */
	mutex_init(&manager->lock);
	/* Misc: initialize */
	manager->misc.name  = DEV_NAME;
	manager->misc.minor = MISC_DYNAMIC_MINOR;
	manager->misc.fops  = &CMA_fops;

	/* list: initialize */
	INIT_LIST_HEAD(&manager->head);

	/* Register Misc device */
	rvl = misc_register(&manager->misc);
	if (rvl) {
		dev_err(&pdev->dev, "Unable to register misc.\n");
		goto err_misc;
	}

	manager->dev = &pdev->dev;
	/* Set platform drvdata */
	platform_set_drvdata(pdev, manager);

	return 0;

err_misc:
	device_remove_file(&pdev->dev, &bitmap_attr);
err_file:
	kfree(manager);
err_alloc:
	return rvl;
}

/* Module exit entry */
static int __exit CMA_remove(struct platform_device *pdev)
{
	struct CMA_region *reg;

	/* Free all region */
	mutex_lock(&manager->lock);
	list_for_each_entry(reg, &manager->head, list)
		kfree(reg);
	mutex_unlock(&manager->lock);

	/* Un-Register Misc device */
	misc_deregister(&manager->misc);
	device_remove_file(&pdev->dev, &bitmap_attr);
	/* free memory */
	kfree(manager);
	manager = NULL;
	return 0;
}

/* file operations */
static struct file_operations CMA_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= CMA_ioctl,
	.mmap		= CMA_mmap,
};

static const struct of_device_id CMA_of_match[] = {
	{ .compatible = "CMA, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, CMA_of_match);

/* Platform Driver Information */
static struct platform_driver CMA_driver = {
	.probe    = CMA_probe,
	.remove   = CMA_remove,
	.driver = {
		.owner  = THIS_MODULE,
		.name   = DEV_NAME,
		.of_match_table = CMA_of_match,
	},
};
module_platform_driver(CMA_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Contiguous Memory Allocate (CMA) Debris issue");
