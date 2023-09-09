// SPDX-License-Identifier: GPL-2.0-only
/*
 * PageFault With Hugefault(FILE PMDMAPPED)
 *
 * (C) 2023.09.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <linux/rmap.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-HUGEPF"

static int BiscuitOS_pud_entry(pud_t *pud, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	struct page *page = (struct page *)walk->private;
	struct vm_area_struct *vma = walk->vma;
	unsigned long pfn = page_to_pfn(page);
	spinlock_t *ptl;
	pmd_t *pmd;

	/* Check PMD Entry is Empty, Address must alignment 2MiB */
	pmd = pmd_offset(pud, addr);
	if (!pmd_none(*pmd))
		return -EINVAL;

	ptl = pmd_lock(vma->vm_mm, pmd);
	/* Setup Pagetable for 2MiB */
	set_pmd_at(vma->vm_mm, addr, pmd,
		pmd_mkhuge(pfn_pmd(pfn, vma->vm_page_prot)));
	__mod_lruvec_page_state(page, NR_FILE_PMDMAPPED, PMD_SIZE / PAGE_SIZE);
	spin_unlock(ptl);
	
	printk("Build 2MiB Page Mapping.\n");

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pud_entry	= BiscuitOS_pud_entry,
};

static vm_fault_t vm_huge_fault(struct vm_fault *vmf, 
				enum page_entry_size pe_size)
{
	struct page *page = alloc_pages(GFP_KERNEL, 9);

	walk_page_vma(vmf->vma, &BiscuitOS_pwalk_ops, (void *)page);	
	/* bind fault page */
	vmf->page = page;
	
	return 0;
}

static const struct vm_operations_struct BiscuitOS_vm_ops = {
	.huge_fault	= vm_huge_fault,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* setup vm_ops */
	vma->vm_ops = &BiscuitOS_vm_ops;
	/* fake DAX */
	filp->f_inode->i_flags |= S_DAX;

	return 0;
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp,
                unsigned long uaddr, unsigned long len,
                unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
			len + HPAGE_SIZE, 0, flags);
	/* Aligned on 2MiB */
	align_addr = round_up(align_addr, HPAGE_SIZE);

	return align_addr;
}

static struct file_operations BiscuitOS_fops = {
	.owner             = THIS_MODULE,
	.mmap              = BiscuitOS_mmap,
	.get_unmapped_area = BiscuitOS_get_unmapped_area,
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
__initcall(BiscuitOS_init);
