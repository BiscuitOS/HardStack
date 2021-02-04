/*
 * CR3 on 32-bit Paging mode
 *
 * (C) 2021.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/slab.h>
#include <linux/uaccess.h>
#ifdef __i386__
#include <linux/highmem.h>
#endif

/* Paging/fault header*/
#include <linux/mm.h>
#include <asm/paravirt.h>

/* DD Platform Name */
#define DEV_NAME			"BiscuitOS"

/* Timer */
#define BISCUITOS_SCANNER_PERIOD	1000 /* 1000ms -> 1s */
static struct timer_list BiscuitOS_scanner;

/* Special fault address */
static unsigned long BiscuitOS_address;
static struct mm_struct *BiscuitOS_mm;
static unsigned long page_fault_cr3;

/* follow CR3 */
static int BiscuitOS_follow_cr3(struct mm_struct *mm, unsigned long address)
{
	phys_addr_t pgd_addr;
	pgd_t *pgd;

	/* pgd */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* load physical address */
	pgd_addr = __pa(pgd) & PAGE_MASK;

	printk("The Virtual  Address of PGD: %#lx\n", (unsigned long)pgd);
	printk("The Physical Address of PGD: %#lx\n", pgd_addr);
	printk("The Value of CR3:            %#lx\n", page_fault_cr3);

	return 0;
out:
	return -EINVAL;
}

/* CR3 Scanner */
static void BiscuitOS_scanner_CR3(struct timer_list *unused)
{
	unsigned long address = BiscuitOS_address;

	if (BiscuitOS_address)
		BiscuitOS_follow_cr3(BiscuitOS_mm, address);

	mod_timer(&BiscuitOS_scanner, 
			jiffies + msecs_to_jiffies(BISCUITOS_SCANNER_PERIOD));
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	spinlock_t *ptl;
	pte_t *pte;
	int r;

	/* Allocate new page from buddy */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System doesn't has enough physical memory.\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* Establish pte and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page))
		return -EAGAIN;

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* bind fault page */
	vmf->page = fault_page;
	/* Special address */
	BiscuitOS_address = address;
	BiscuitOS_mm = vma->vm_mm;
	page_fault_cr3 = __read_cr3();

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

	/* Timer for PTE Scanner */
	timer_setup(&BiscuitOS_scanner, BiscuitOS_scanner_CR3, 0);
	mod_timer(&BiscuitOS_scanner, 
			jiffies + msecs_to_jiffies(BISCUITOS_SCANNER_PERIOD));

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	del_timer(&BiscuitOS_scanner);
	/* Un-Register Misc device */
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Paging/Page-fault Mechanism");
