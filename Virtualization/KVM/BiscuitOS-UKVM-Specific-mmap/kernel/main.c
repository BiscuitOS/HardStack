/*
 * KVM Specific Private MMAP on BiscuitOS
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS"
/* Private page */
static struct page *page_map;

/* MMAP on BiscuitOS */
static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	ssize_t size = PAGE_ALIGN(vma->vm_end - vma->vm_start);
	int npages = size / PAGE_SIZE;

	/* Allocate page */
	page_map = alloc_pages(GFP_KERNEL, get_order(npages));
	if (!page_map) {
		printk("ERROR: No free memory on alloc page.\n");
		return -ENOMEM;
	}

	/* Private mmap */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    page_to_pfn(page_map),
			    vma->vm_end - vma->vm_start,
			    vma->vm_page_prot)) {
		printk("ERROR: Remap failed.\n");
		return -EAGAIN;
	}

	return 0;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);
	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS KVM Private MMAP");
