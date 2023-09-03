// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault ERROR CODE: PF_READ
 *
 * (C) 2023.09.03 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <asm/pgtable.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define _PAGE_RSVD		(_AT(pteval_t, 1) << 51)

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	struct page *fault_page = (struct page *)walk->private;
	unsigned long pfn = page_to_pfn(fault_page);
	struct vm_area_struct *vma = walk->vma;
	spinlock_t *ptl;
	pte_t *pte;

	/* Check PTE Entry is Empty, Address must alignment 4KiB */
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	if (!pte_none(*pte)) {
		pte_unmap_unlock(pte, ptl);
		return -EINVAL;
	}

	mm_inc_nr_ptes(vma->vm_mm);
	smp_wmb();
	/* Setup Pagetable for 4KiB, SET RSVD Bit  */
	set_pte_at(vma->vm_mm, addr, pte,
		pte_set_flags(pte_mkwrite(
		pfn_pte(pfn, vma->vm_page_prot)), _PAGE_RSVD));
	pte_unmap_unlock(pte, ptl);
	printk("Build 4KiB Page Mapping.\n");

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pmd_entry	= BiscuitOS_pmd_entry,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct page *fault_page;

	fault_page = alloc_page(GFP_KERNEL);
	if (!fault_page) {
		printk("ERROR: System doesn't has physical memory.\n");
		return -ENOMEM;
	}
	atomic_inc(&fault_page->_refcount);
	atomic_inc(&fault_page->_mapcount);

	return walk_page_vma(vma, &BiscuitOS_pwalk_ops, (void *)fault_page);
}

static struct file_operations BiscuitOS_fops = {
	.owner			= THIS_MODULE,
	.mmap			= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= SPECIAL_DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	return misc_register(&BiscuitOS_drv);
}
__initcall(BiscuitOS_init);
