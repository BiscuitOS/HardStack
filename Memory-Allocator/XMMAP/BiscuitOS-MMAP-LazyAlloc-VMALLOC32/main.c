/*
 * Lazy Alloc Memory from VMALLOC32 Allocator
 *
 * (C) 2023.02.14 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

/* DD Platform Name */
#define SPECIAL_DEV_NAME	"BiscuitOS"

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	void *mem;

	mem = vmalloc_32_user(PAGE_SIZE);
	if (!mem) {
		printk("System Error: No free VMALLOC memory!\n");
		return -EAGAIN;
	}

	/* VMA must contain VM_MIXEDMAP on Page-Fault route */
	vma->vm_flags |= VM_MIXEDMAP;
	/* Establish User Pagetable */
	remap_vmalloc_range(vma, mem, 0);

	/* bind fault page */
	vmf->page = vmalloc_to_page(mem);

	return 0;
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

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
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
MODULE_DESCRIPTION("Lazy Alloc from VMALLOC32");
