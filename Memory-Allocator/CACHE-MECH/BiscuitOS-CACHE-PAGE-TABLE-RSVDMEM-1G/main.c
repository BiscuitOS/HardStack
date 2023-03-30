// SPDX-License-Identifier: GPL-2.0
/*
 * RSVDMEM 1Gig with variable Memory Type to Userspace
 *  CMDLINE: 'memmap=1G$0x100000000'
 *  Enable Kernel Macro: CONFIG_TRANSPARENT_HUGEPAGE
 *  System Ram must big than 4Gig
 *
 * (C) 2023.02.14 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <asm/pgalloc.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-MEM-1G"
#define RSVDMEM_PFN		0x100000

static int BiscuitOS_p4d_entry(p4d_t *p4d, unsigned long addr,
				unsigned long next, struct mm_walk *walk)
{
	struct vm_area_struct *vma = walk->vma;
	enum page_cache_mode pcm = vma->vm_pgoff; /* PAT from pgoff */
	spinlock_t *ptl;
	pgprot_t prot;
	pud_t *pud;

	/* Check PUD Entry is empty, Address must aligned 1Gig */
	pud = pud_offset(p4d, addr);
	if (!pud_none(*pud))
		return -EINVAL;

	ptl = pud_lock(walk->vma->vm_mm, pud);
	/* Clear PAT Attribute */
	pgprot_val(vma->vm_page_prot) &= ~(_PAGE_PAT | _PAGE_PCD | _PAGE_PWT);
	pgprot_val(vma->vm_page_prot) |= cachemode2protval(pcm);
	prot = pgprot_4k_2_large(vma->vm_page_prot);
	/* Mark VM */
	vma->vm_flags |= VM_MIXEDMAP | VM_PAT;

	/* Setup PageTable Attribute */
	pgprot_val(prot) |= _PAGE_RW | _PAGE_DEVMAP;
	/* Check Memory Type */
	if (track_pfn_remap(vma, &prot, RSVDMEM_PFN, addr, PUD_PAGE_SIZE)) {
		spin_unlock(ptl);
		return -EINVAL;
	}

	/* Setup Pagetable for 1Gig */
	set_pud_at(vma->vm_mm, addr, pud,
			pud_mkhuge(pfn_pud(RSVDMEM_PFN, prot)));
	spin_unlock(ptl);

	return 1; /* Stop walk */
}

static const struct mm_walk_ops BiscuitOS_walk_ops = {
	.p4d_entry = BiscuitOS_p4d_entry,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	walk_page_vma(vma, &BiscuitOS_walk_ops, NULL);
	return 0;
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp, 
			unsigned long uaddr, unsigned long len,
			unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
					len + PUD_PAGE_SIZE, 0, flags);
	/* Aligned on 1Gig */
	align_addr = round_up(align_addr, PUD_PAGE_SIZE); 

	return align_addr;
}

/* file operations */
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
	misc_register(&BiscuitOS_drv);

	return 0;
}
device_initcall(BiscuitOS_init);
