/*
 * Paging Mechanism: Mapping 4KiB Page With 32-Bit Paging
 *
 * (C) 2021.01.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __i386__
#error "This Code only running on Intl-i386 Architecture!"
#endif

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>

/* Paging/fault header*/
#include <linux/mm.h>
#include <linux/kallsyms.h>

/* DD Platform Name */
#define DEV_NAME			"BiscuitOS"

/* Timer */
#define BISCUITOS_SCANNER_PERIOD	1000 /* 1000ms -> 1s */
static struct timer_list BiscuitOS_scanner;

/* Speical */
static struct vm_area_struct *BiscuitOS_vma;
unsigned long BiscuitOS_address;

/* kallsyms unexport symbol */
typedef int (*__pte_alloc_t)(struct mm_struct *, pmd_t *);
typedef void (*page_add_f_rmap_t)(struct page *, bool);

static __pte_alloc_t __pte_alloc_func;
static page_add_f_rmap_t page_add_file_rmap_func;

/* follow pte */
static int BiscuitOS_follow_page_table(struct mm_struct *mm, 
		unsigned long address, pte_t **ptep, spinlock_t **ptl)
{
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	/* Follow PGD Entry */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* PDE */
	pde = (pmd_t *)pgd;

	/* Follow PTE */
	pte = pte_offset_map_lock(mm, pde, address, ptl);
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

/* Build Page table */
static int BiscuitOS_build_page_table(struct vm_area_struct *vma, 
				unsigned long address, struct page *page)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long pfn = page_to_pfn(page);
	spinlock_t *ptl;
	pgd_t *pgd;
	pmd_t *pde;
	pte_t *pte;

	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* PDE */
	pde = (pmd_t *)pgd;

	/* alloc pte */
	pte = __pte_alloc_func(mm, pde) ? 
		NULL : pte_offset_map_lock(mm, pde, address, &ptl);
	if (!pte)
		goto out;

	/* MMU Lazy mode */
	arch_enter_lazy_mmu_mode();

	get_page(page);
	inc_mm_counter(mm, mm_counter_file(page));
	page_add_file_rmap_func(page, false);

	set_pte_at(mm, address, pte, 
			pte_mkwrite(pfn_pte(pfn, vma->vm_page_prot)));
	pte_unmap_unlock(pte, ptl);
	
	arch_leave_lazy_mmu_mode();

	return 0;

out:
	return -EINVAL;
}

/* PTE Scanner */
static void BiscuitOS_scanner_pte(struct timer_list *unused)
{
	if (BiscuitOS_address) {
		spinlock_t *ptl;
		pte_t *pte;

		/* follow page table */
		BiscuitOS_follow_page_table(BiscuitOS_vma->vm_mm,
				BiscuitOS_address, &pte, &ptl);
		if (pte && pte_present(*pte)) {
			struct page * page;
			unsigned long pfn;

			page = pte_page(*pte);
			pfn = page_to_pfn(page);

			printk("Page %#lx PTE %#lx\n", pfn, pte_val(*pte));
			pte_unmap_unlock(pte, ptl);
		}

		BiscuitOS_address = 0;
	}

	/* watchdog */
	mod_timer(&BiscuitOS_scanner, 
			jiffies + msecs_to_jiffies(BISCUITOS_SCANNER_PERIOD));
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	unsigned long address = vmf->address;
	struct page *fault_page;
	int r;

	/* Allocate new page from buddy */
	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System doesn't has enough physical memory.\n");
		r = -ENOMEM;
		goto err_alloc;
	}

	/* Build page table */
	BiscuitOS_build_page_table(vma, address, fault_page);

	/* bind fault page */
	vmf->page = fault_page;

	/* bind special data */
	BiscuitOS_vma = vma;
	BiscuitOS_address = address;

	printk("Page Fault: %#lx\n", page_to_pfn(fault_page));
	return 0;

err_alloc:
	return r;
}

static inline void init_symbol(void)
{
	__pte_alloc_func = 
		(__pte_alloc_t)kallsyms_lookup_name("__pte_alloc");
	page_add_file_rmap_func = 
		(page_add_f_rmap_t)kallsyms_lookup_name("page_add_file_rmap");
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
	init_symbol();

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
