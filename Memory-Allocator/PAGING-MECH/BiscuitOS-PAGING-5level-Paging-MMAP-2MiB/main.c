// SPDX-License-Identifier: GPL-2.0
/*
 * 5-level Paging Mapping 2MiB
 *
 * (C) 2023.07.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <asm/pgtable.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"

static int BiscuitOS_pud_entry(pud_t *pud, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	struct page *fault_page = (struct page *)walk->private;
	unsigned long pfn = page_to_pfn(fault_page);
	struct vm_area_struct *vma = walk->vma;
	spinlock_t *ptl;
	pmd_t *pmd;

	/* Check PMD Entry is Empty, Address must alignment 2MiB */
	pmd = pmd_offset(pud, addr);
	if (!pmd_none(*pmd))
		return -EINVAL;

	ptl = pmd_lock(vma->vm_mm, pmd);
	mm_inc_nr_pmds(vma->vm_mm);
	smp_wmb();
	/* Setup Pagetable for 2MiB */
	set_pmd_at(vma->vm_mm, addr, pmd,
		pmd_mkhuge(pfn_pmd(pfn, vma->vm_page_prot)));
	spin_unlock(ptl);
	printk("Build 2MiB Page Mapping.\n");

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pud_entry	= BiscuitOS_pud_entry,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct page *fault_page;

	fault_page = alloc_pages(GFP_KERNEL, 9);
	if (!fault_page) {
		printk("ERROR: System doesn't has enough physical memory.\n");
		return -ENOMEM;
	}
	atomic_inc(&fault_page->_refcount);
	atomic_inc(&fault_page->_mapcount);

	return walk_page_vma(vma, &BiscuitOS_pwalk_ops, (void *)fault_page);
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp,
		unsigned long uaddr, unsigned long len,
		unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
			len + HPAGE_SIZE, 0, flags);
	/* Aligned on 4MiB */
	align_addr = round_up(align_addr, HPAGE_SIZE);

	return align_addr;
}

static struct file_operations BiscuitOS_fops = {
	.owner			= THIS_MODULE,
	.mmap			= BiscuitOS_mmap,
	.get_unmapped_area	= BiscuitOS_get_unmapped_area,
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
device_initcall(BiscuitOS_init);
