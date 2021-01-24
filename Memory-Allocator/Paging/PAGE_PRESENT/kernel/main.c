/*
 * Paging Mechanism PTE Present "P"
 *
 * (C) 2021.01.20 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* DD Platform Name */
#define DEV_NAME			"BiscuitOS"

/* Timer */
#define BISCUITOS_SCANNER_PERIOD	1000 /* 1000ms -> 1s */
static struct timer_list BiscuitOS_scanner;

/* Special PTE */
static pte_t *BiscuitOS_pte;
struct mm_struct *BiscuitOS_mm;

/* follow pte */
static int BiscuitOS_get_locked_pte(struct mm_struct *mm, 
		unsigned long address, pte_t **ptep, spinlock_t **ptl)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	/* Follow PGD Entry */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* Follow P4D Entry */
	p4d = p4d_offset(pgd, address);
	if (p4d_none(*p4d) || unlikely(p4d_bad(*p4d)))
		goto out;

	/* Follow PUD Entry */
	pud = pud_offset(p4d, address);
	if (pud_none(*pud) || unlikely(pud_bad(*pud)))
		goto out;

	/* Follow PMD Entry */
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
		goto out;

	/* Follow PTE */
	pte = pte_offset_map_lock(mm, pmd, address, ptl);
	if (!pte)
		goto out;
	if (!pte_present(*pte))
		goto unlock;
	*ptep = pte;
	return 0;

unlock:
	pte_unmap_unlock(ptep, *ptl);
out:
	return -EINVAL;
}

/* PTE Scanner */
static void BiscuitOS_scanner_pte(struct timer_list *unused)
{
	/* Check PTE */
	if (BiscuitOS_pte && pte_present(*BiscuitOS_pte)) {
		struct page *page;
		unsigned long pfn;

		page = pte_page(*BiscuitOS_pte);
		pfn = pte_pfn(*BiscuitOS_pte);
	}

	/* PTE _PAGE_PRESENT */
	if (pte_flags(*BiscuitOS_pte) & _PAGE_PRESENT)
		printk("PTE _PAGE_PRESENT Bit Set.\n");
	else
		printk("PTE _PAGE_PRESENT Bit Clear.\n");

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
	trace_printk("Page-Fault: PFN %#lx\n", page_to_pfn(fault_page));

	/* Establish pte and INC _mapcount for page */
	vma->vm_flags |= VM_MIXEDMAP;
	if (vm_insert_page(vma, address, fault_page))
		return -EAGAIN;

	/* Add refcount for page */
	atomic_inc(&fault_page->_refcount);
	/* bind fault page */
	vmf->page = fault_page;

	/* Special pte */
	BiscuitOS_get_locked_pte(vma->vm_mm, address, &pte, &ptl);
	if (pte && pte_present(*pte)) {
		pte_unmap_unlock(pte, ptl);
		BiscuitOS_pte = pte;
		BiscuitOS_mm = vma->vm_mm;
	}

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
	timer_setup(&BiscuitOS_scanner, BiscuitOS_scanner_pte, 0);
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
