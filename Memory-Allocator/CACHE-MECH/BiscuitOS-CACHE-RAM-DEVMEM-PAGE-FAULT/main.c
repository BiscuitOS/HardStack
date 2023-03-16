/*
 * DEVMEM Page-fault with Memory Type
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
#include <linux/mm.h>
#include <asm/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-MEM"

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	unsigned long pfn;
	int r;

	/* Allocate Page as DEVMEM */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: NO Free Memory from DEVMEM.\n");
		r = -ENOMEM;
		goto err_alloc;
	}
	pfn = page_to_pfn(fault_page);

	/* Clear PAT Attribute */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PCD | _PAGE_PWT | _PAGE_PAT);

	/* Change Memory Type for Direct-Mapping Area */
	arch_io_reserve_memtype_wc(PFN_PHYS(pfn), PAGE_SIZE);
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(_PAGE_CACHE_MODE_WC);

	/* Establish pte and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page))
		return -EAGAIN;

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* bind fault page */
	vmf->page = fault_page;

	return 0;

err_alloc:
	return r;
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.fault  = vm_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("DEVMEM Page-Fault with CACHE MODE");
