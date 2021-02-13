/*
 * Paging Mechanism: Mapping 4MiB Page With 32-Bit Paging
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

/* Page Table flags */
#define _PAGE_4M_USER			(_PAGE_PRESENT | _PAGE_RW | \
					 _PAGE_ACCESSED | _PAGE_DIRTY | \
					 _PAGE_ENC | _PAGE_USER | \
					 _PAGE_PSE)

/* Timer */
#define BISCUITOS_SCANNER_PERIOD	1000 /* 1000ms -> 1s */
static struct timer_list BiscuitOS_scanner;

/* Speical */
static struct vm_area_struct *BiscuitOS_vma;
unsigned long BiscuitOS_address;

/* kallsyms unexport symbol */
typedef int (*__pte_alloc_t)(struct mm_struct *, pmd_t *);
typedef void (*page_add_f_rmap_t)(struct page *, bool);
typedef void (*flush_tlb_mm_range_t)(struct mm_struct *, unsigned long,
				unsigned long, unsigned int, bool);

static __pte_alloc_t __pte_alloc_func;
static page_add_f_rmap_t page_add_file_rmap_func;
static flush_tlb_mm_range_t flush_tlb_mm_range_func;

/* Follow PDE */
static int BiscuitOS_follow_page_table(struct mm_struct *mm, 
				unsigned long address, pmd_t **pdep)
{
	pgd_t *pgd;
	pmd_t *pde;

	/* Follow PGD Entry */
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* PDE */
	pde = (pmd_t *)pgd;

	*pdep = pde;
	return 0;

out:
	return -EINVAL;
}

/* Build Page table */
static int BiscuitOS_build_page_table(struct vm_area_struct *vma, 
				unsigned long address, struct page *page)
{
	unsigned long end = address + (1 << 22);
	unsigned long pfn = page_to_pfn(page);
	struct mm_struct *mm = vma->vm_mm;
	pgd_t *pgd;
	pmd_t *pde;

	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
		goto out;

	/* PDE */
	pde = (pmd_t *)pgd;

	/* MMU Lazy mode */
	arch_enter_lazy_mmu_mode();

	get_page(page);
	inc_mm_counter(mm, mm_counter_file(page));
	page_add_file_rmap_func(page, false);

	/* PDE populate */
	set_pmd(pde, pfn_pmd(pfn, __pgprot(_PAGE_4M_USER)));
	flush_tlb_mm_range_func(vma->vm_mm, address, end, PAGE_SHIFT, false);
	
	arch_leave_lazy_mmu_mode();

	return 0;

out:
	return -EINVAL;
}

/* PDE Scanner */
static void BiscuitOS_scanner_pte(struct timer_list *unused)
{
	if (BiscuitOS_address) {
		pmd_t *pde;

		BiscuitOS_follow_page_table(BiscuitOS_vma->vm_mm, 
						BiscuitOS_address, &pde);
		if (pde && pmd_present(*pde)) {
			printk("PMD %#lx PFN %#lx\n", pmd_val(*pde),
							pmd_pfn(*pde));
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
	fault_page = alloc_pages(GFP_KERNEL, 10);
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
	flush_tlb_mm_range_func =
	     (flush_tlb_mm_range_t)kallsyms_lookup_name("flush_tlb_mm_range");
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
