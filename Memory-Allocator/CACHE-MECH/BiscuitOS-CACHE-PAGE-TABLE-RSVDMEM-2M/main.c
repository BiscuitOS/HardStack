// SPDX-License-Identifier: GPL-2.0
/*
 * RSVDMEM 2MiB with variable Memory Type
 *  CMDLINE: 'memmap=2M$0x10000000'
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

#define SPECIAL_DEV_NAME	"BiscuitOS-MEM-2M"
#define RSVDMEM_BASE		0x10000000

static int BiscuitOS_pud_entry(pud_t *pud, unsigned long addr,
				unsigned long next, struct mm_walk *walk)
{
	enum page_cache_mode pcm = walk->vma->vm_pgoff; /* PAT from pgoff */
	spinlock_t *ptl;
	pgprot_t prot;
	pmd_t *pmd;

	/* Check PMD Entry is empty */
	pmd = pmd_offset(pud, addr);
	if (!pmd_none(*pmd))
		return -EINVAL;

	ptl = pmd_lock(walk->vma->vm_mm, pmd);
	/* Clear PAT Attribute */
	prot = walk->vma->vm_page_prot;
	pgprot_val(prot) &= ~(_PAGE_PAT | _PAGE_PCD | _PAGE_PWT);

	/* Setup PageTable Attribute */
	pgprot_val(prot) |= _PAGE_RW | _PAGE_USER | _PAGE_PRESENT;
	pgprot_val(prot) |= cachemode2protval(pcm);

	prot = pgprot_4k_2_large(prot);
	/* Setup Pagetable for 2MiB */
	set_pmd_at(walk->vma->vm_mm, addr, pmd,
		pmd_mkhuge(pfn_pmd(RSVDMEM_BASE >> PAGE_SHIFT, prot)));
	spin_unlock(ptl);

	return 1; /* Stop walk */
}

static const struct mm_walk_ops BiscuitOS_walk_ops = {
	.pud_entry = BiscuitOS_pud_entry,
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
					len + HPAGE_SIZE, 0, flags);
	/* Aligned on 2MiB */
	align_addr = round_up(align_addr, HPAGE_SIZE); 

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
