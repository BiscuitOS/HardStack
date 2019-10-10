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

/* LDD Driver Name */
#define DEV_NAME		"CMA_demo"
#define CMA_REGION_NUM		64
#define CMA_MEM_ALLOCATE	_IOW('m', 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW('m', 2, unsigned int)


/* CMA region information */
struct CMA_demo_region {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
	struct list_head list;
};

/* CMA manager information */
struct CMA_demo_manager {
	struct miscdevice misc;
	struct mutex lock;
	struct list_head head;
};

/* CMA memory device */
static struct CMA_demo_manager *manager;

static long CMA_demo_ioctl(struct file *filp, unsigned int cmd, 
							unsigned long arg)
{
	struct CMA_demo_region region;
	struct CMA_demo_region *reg;
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
		if (copy_from_user(&region, (void __user *)arg,
					sizeof(struct CMA_demo_region))) {
			printk(KERN_ERR "ALLOCATE: copy_from_user error\n");
			rvl = -EFAULT;
			goto err_user;
		}

		/* allocate new region */
		reg = kzalloc(sizeof(*reg), GFP_KERNEL);
		if (!reg) {
			printk(KERN_ERR "ALLOCATE: no free memory.\n");
			rvl = -ENOMEM;
			goto err_alloc;
		}

		nr_pages = region.length >> PAGE_SHIFT;
		pool_size_order = get_order(region.length);
		/* Allocate memory from CMA */
		page = dma_alloc_from_contiguous(NULL, nr_pages,
				pool_size_order, GFP_KERNEL);
		if (!page) {
			printk(KERN_ERR "ALLOCATE: DMA allocate error\n");
			rvl = -ENOMEM;
			goto err_dma;
		}
		
		/* Insert region into manager */
		reg->virt = (dma_addr_t)page_to_virt(page);
		reg->phys = (dma_addr_t)page_to_phys(page);
		reg->length = region.length;
		list_add(&reg->list, &manager->head);

		/* export to userland */
		if (copy_to_user((void __user *)arg, reg, sizeof(*reg))) {
			printk(KERN_ERR "ALLOCATE: copy_to_user error\n");
			rvl = -EINVAL;
			goto err_to;
		}
		list_for_each_entry(reg, &manager->head, list)
			printk("Region: %#lx - %#lx\n", reg->phys,
								reg->length);
		/* unlock */
		mutex_unlock(&manager->lock);
		return 0;
	case CMA_MEM_RELEASE:
		mutex_lock(&manager->lock);
		if (copy_from_user(&region, (void __user *)arg,
							sizeof(region))) {
			printk(KERN_ERR "RELEASE: copy_from_user\n");
			rvl = -EINVAL;
			goto err_user;
		}
		/* Search region */
		list_for_each_entry(reg, &manager->head, list) {
			if (reg->phys == region.phys &&
				     reg->length == region.length) {
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
		page = phys_to_page(region.phys);
		nr_pages = region.length >> PAGE_SHIFT;
		dma_release_from_contiguous(NULL, page, nr_pages);
		list_del(&reg->list);
		kfree(reg);
		mutex_unlock(&manager->lock);

		return 0;
	default:
		printk(KERN_INFO "CMA not support command.\n");
		return -EFAULT;
	}

	return 0;

err_to:
	list_del(&reg->list);
	dma_release_from_contiguous(NULL, page, nr_pages);
err_dma:
	kfree(reg);
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
	unsigned long page, pos;

	printk("START: %#lx\n", start);

	return 0;
}

/* file operations */
static struct file_operations CMA_demo_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= CMA_demo_ioctl,
	.mmap		= CMA_demo_mmap,
};

/* Module initialize entry */
static int __init CMA_demo_init(void)
{
	int rvl;

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
	return 0;

err_alloc:
	return rvl;
}

/* Module exit entry */
static void __exit CMA_demo_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&manager->misc);
	/* free memory */
	kfree(manager);
	manager = NULL;
}

module_init(CMA_demo_init);
module_exit(CMA_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CMA Device Driver");
