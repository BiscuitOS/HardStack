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
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/dma-contiguous.h>

#include <asm/uaccess.h>

/* LDD Driver Name */
#define DEV_NAME "CMA_demo"
#define CMA_MEM_ALLOCATE	_IOW("m", 1, unsigned int)
#define CMA_MEM_RELEASE		_IOW("m", 2, unsigned int)
#define CMA_REGION_NUM		64

/* CMA region information */
struct CMA_demo_region {
	unsigned long virt;
	unsigned long phys;
	unsigned long offset;
	unsigned long length;
};

/* CMA manager information */
struct CMA_demo_manager {
	struct miscdevice misc;
	struct mutex lock;
	unsigned long nr_regions;
	struct CMA_demo_region regions[CMA_REGION_NUM];
};

/* CMA memory device */
static struct CMA_demo_manager *manager;

enum region_pos {
	REGION_ISOLATE	= 0,
	REGION_OVERLAP	= 1,
	REGION_CONTAIN	= 2,
	REGION_COINCID	= 3,
	REGION_TOTAL,
	REGION_UNKNOW,
};

#define for_each_region(idx, np, manager) \
	for (idx = 0, np = manager->regions[0]; \
			idx < manager->nr_regions; \
				idx++, np = manager->regions[idx])

/* Check Region position
 *
 * @Origion: Default region
 * @region:  Check region
 *
 * @return: 0 - Isolate     - REGION_ISOLATE
 *          1 - Overlap     - REGION_OVERLAP
 *          2 - Contain     - REGION_CONTAIN
 *          3 - Coincidence - REGION_COINCID
 *
 */
static inline int check_region_pos(struct CMA_demo_region *Origin, 
				 struct CMA_demo_region *region)
{
	unsigned long Oend = Origin->phys + Origin->length;
	unsigned long Rend = region->phys + region->length;
	unsigned long Obegin = Origion->phys;
	unsigned long Rbegin = region->phys;

	/* Region Isolate
	 *
	 *      Origin
	 * | <------------> |
	 * +----------------+---------------+----------------+
	 * |                |               |                |
	 * +----------------+---------------+----------------+
	 *                                  | <------------> |
	 *                                         Region
	 *       Region
	 * | <------------> |
	 * +----------------+---------------+----------------+
	 * |                |               |                |
	 * +----------------+---------------+----------------+
	 *                                  | <------------> |
	 *                                        Origin
	 *
	 *        Origin            Region
	 * | <--------------> | <-------------> |
	 * +-------------------------------------------------+
	 * |                  |                 |            |
	 * +-------------------------------------------------+
	 *
	 *        Region            Origin
	 * | <--------------> | <-------------> |
	 * +-------------------------------------------------+
	 * |                  |                 |            |
	 * +-------------------------------------------------+
	 */
	if (Oend <= Rbegin || Rend <= Obegin)
		return REGION_ISOLATE;

	/* Region Contain
	 *
	 *               Region
	 * | <------------------------------> |
	 * +----------------+-----------------+--------------+
	 * |                |                 |              |
	 * +----------------+-----------------+--------------+
	 *                  | <-------------> |
	 *                        Origin
	 *
	 *
	 *                      Region
	 *         | <--------------------------> |
	 * +-------+---------------+--------------+----------+
	 * |       |               |              |          |
	 * +-------+---------------+--------------+----------+
	 *         | <-----------> |          
	 *              Origion
	 *
	 *
	 *               Origion
	 * | <------------------------------> |
	 * +----------------+-----------------+--------------+
	 * |                |                 |              |
	 * +----------------+-----------------+--------------+
	 *                  | <-------------> |
	 *                        Region
	 *
	 *
	 *                      Origion
	 *         | <--------------------------> |
	 * +-------+---------------+--------------+----------+
	 * |       |               |              |          |
	 * +-------+---------------+--------------+----------+
	 *         | <-----------> |          
	 *              Region
	 */
	if ((Rend == Oend && Rbegin != Obegin) || 
			(Rbegin == Obegin && Oend != Rend))
		return REGION_CONTAIN;

	/* Region Coincidence
	 *
	 *                   Region
	 *          | <--------------------> |
	 * +--------+------------------------+---------------+
	 * |        |                        |               |
	 * +--------+------------------------+---------------+
	 *          | <--------------------> |
	 *                   Origin
	 */
	if (Rbegin == Obegin && Rend == Oend)
		return REGION_COINCID;

	/* Region Overlap
	 *
	 *             Region
	 * | <----------------------> |
	 * +---------------+----------+------------+---------+
	 * |               |          |            |         |
	 * +---------------+----------+------------+---------+
	 *                 | <-------------------> |
	 *                          Origin
	 *
	 *             Origin
	 * | <----------------------> |
	 * +---------------+----------+------------+---------+
	 * |               |          |            |         |
	 * +---------------+----------+------------+---------+
	 *                 | <-------------------> |
	 *                          Region
	 */
	if (Rbegin != Obegin && Rend != Oend)
		return REGION_OVERLAP;
	return REGION_UNKNOWN;
}

static long CMA_demo_ioctl(struct file *filp, unsigned int cmd, 
							unsigned long arg)
{
	struct CMA_demo_region region;
	struct CMA_demo_region *reg;
	struct CMA_demo_region *np;
	unsigned int pool_size_order;
	unsigned long nr_pages;
	struct page *page;
	int found = 0;

	switch (cmd) {
	case CMA_MEM_ALLOCATE:
		reg = kzalloc(sizeof(struct CMA_demo_region), GFP_KERNEL);
		if (!reg) {
			printk(KERN_ERR "ALLOCATE: no free memory\n");
			goto cma_region;
		}

		/* CMA allocate interface */
		mutex_lock(&manager->lock);
		if (copy_from_user(&region, (void __user *)arg,
					sizeof(struct CMA_demo_region))) {
			printk(KERN_ERR "ALLOCATE: copy error\n");
			goto cma_fail;
		}

		nr_pages = region.length >> PAGE_SHIFT;
		pool_size_order = get_order(region.length);
		/* Allocate memory from CMA */
		page = dma_alloc_from_contiguous(NULL, nr_pages,
					pool_size_order, GFP_KERNEL);
		if (!page) {
			printk(KERN_ERR "ALLOCATE: DMA allocate error\n");
			goto cma_fail;
		}

		/* CMAInfo: transfer data into userland */
		region.virt = (dma_addr_t)page_to_virt(page);
		region.phys = (dma_addr_t)page_to_phys(page);
		/* Add CMA manager list */
		if (copy_to_user((void __user *)arg, &region, 
					sizeof(struct CMA_demo_region))) {
			printk(KERN_INFO "ALLOCATE: copy_to_user error\n");
			goto cma_fail;
		}
		mutex_unlock(&manager->lock);
		return 0;
cma_fail:
		kfree(reg);
cma_region:
		mutex_unlock(&manager->lock);
		return -EFAULT;

	case CMA_MEM_RELEASE:
		mutex_lock(&manager->lock);
		if (copy_from_user(&region, (void __user *)arg,
					sizeof(struct CMA_demo_region))) {
			printk(KERN_ERR "RELEASE: copy_from_user error\n");
			goto cma_fault;
		}

		page = phys_to_page(region.phys);
		nr_pages = region.length >> PAGE_SHIFT;
		dma_release_from_contiguous(NULL, page, nr_pages);
		/* Release region */
		kfree(reg);
		mutex_unlock(&manager->lock);
		return 0;
cma_fault:
		mutex_unlock(&manager->lock);
		return -EFAULT;

	default:
		printk(KERN_INFO "CMA not support command.\n");
		return -EFAULT;
	}

	return 0;

}

static int CMA_demo_mmap(struct file *filp, struct vm_area_struct *vma)
{
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
	/* regions: initialize */
	manager->nr_regions = 0;

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
