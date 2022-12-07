/*
 * BiscuitOS Paging Base
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.10.16 BiscuitOS
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
#include <linux/mm_types.h>

#define DEV_NAME		"BiscuitOS"

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	int r;

	/* Allocate Physical Memory */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System Emerge Physical Memory!\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* Modify Page Table Bit */
	printk("Default Pgport %#lx\n", pgprot_val(vma->vm_page_prot));
	pgprot_val(vma->vm_page_prot) |= _PAGE_PWT | _PAGE_PCD | _PAGE_PAT;
	printk("Modify Pgport %#lx\n", pgprot_val(vma->vm_page_prot));

	/* Modify VMA flags */
	printk("Default VMA Flags: %#lx\n", vma->vm_flgas);
	vma->vm_flags &= ~VM_READ;
	printk("Modify VMA Flags: %#lx\n", vma->vm_flags);

	/* Fill PTE and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page))
		return -EAGAIN;

	printk("Correct Pgport %#lx\n", pgprot_val(vma->vm_page_prot));
	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* Bind fault page */
	vmf->page = fault_page;

	return 0;

err_alloc:
	return r;
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
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
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
MODULE_DESCRIPTION("BiscuitOS Paging Mechanism");
