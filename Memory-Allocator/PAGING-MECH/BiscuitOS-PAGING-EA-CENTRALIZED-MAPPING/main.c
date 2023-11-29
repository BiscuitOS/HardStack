// SPDX-License-Identifier: GPL-2.0-only
/*
 * Centralized Mapping: 512G on Signle PMD PAGE
 *
 * (C) 2023.11.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pfn_t.h>
#include <asm/pgtable_types.h>
#include <asm-generic/pgalloc.h>
#include <asm/paravirt.h>
#include <asm/pgtable.h>
#include <linux/mmu_notifier.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-CETMAP"
#define ALIGN_512G		(0x8000000000ULL) /* 512G */

static struct mmu_notifier notifier;

/* MONITOR PGTABLE RELEASE */
static int BiscuitOS_invalidate_range_start(struct mmu_notifier *mni,
		const struct mmu_notifier_range *range)
{
	struct page *page;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;

	if (range->event != MMU_NOTIFY_UNMAP)
		return 0;

	/* CONSULE PGTABLE */
	pgd = pgd_offset(mni->mm, range->start);
	if (pgd_none(*pgd))
		return -EINVAL; /* FORCE FAILED */

	p4d = p4d_offset(pgd, range->start);
	if (p4d_none(*p4d))
		return -EINVAL; /* FORCE FAILED */

	pud = pud_offset(p4d, range->start);
	if (pud_none(*pud))
		return -EINVAL; /* FORCE FAILED */

	pmd = pmd_offset(pud, range->start);
	if (pmd_none(*pmd))
		return -EINVAL; /* FORCE FAILED */

	/* RECLAIM */
	page = pmd_page(*pmd);
	__free_pages(page, 9); /* RELEASE ZERO HUGE PAGE */
	page = pud_page(*pud);
	__free_page(page); /* RELEASE PMD PGTABLE PAGE */
	page = p4d_page(*p4d);
	__free_page(page); /* RELEASE PUD PGTABLE PAGE */

	smp_wmb();
	/* CLEAR AND UPDATE PGTABLE */
	WRITE_ONCE(*p4d, __p4d(0));

	return 0;
}

static const struct mmu_notifier_ops BiscuitOS_mn_ops = {
	.invalidate_range_start = BiscuitOS_invalidate_range_start,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long pud_pg, pmd_pg;
	struct page *page;
	pgd_t *pgd;
	p4d_t *orig_p4d, p4t;
	pud_t put;
	pmd_t pmt;
	int i;

	/* ALLOC PGTABLE FORCE SUCCESS */
	pud_pg = get_zeroed_page(GFP_PGTABLE_USER);
	pmd_pg = get_zeroed_page(GFP_PGTABLE_USER);

	/* BUILD P4D ENTRY */
	p4t = __p4d(_PAGE_TABLE | __pa(pud_pg));
	/* BUILD PUD ENTRY */
	put = __pud(_PAGE_TABLE | __pa(pmd_pg));

	/* PTE/PMD FORCE SUCCESS */
	page = alloc_pages(GFP_ATOMIC | __GFP_ZERO, 9);
	/* BUILD PMD ENTRY */
	pmt = pmd_mkhuge(mk_pmd(page, PAGE_READONLY));

	/* POPULATE PGTABLE */
	for (i = 0; i < PTRS_PER_PTE; i++) {
		pud_t *pud = (pud_t *)pud_pg + i;
		pmd_t *pmd = (pmd_t *)pmd_pg + i;

		/* POPULATE PUD ENTRY */
		WRITE_ONCE(*pud, put);
		/* POPULATE PMD ENTRY */
		WRITE_ONCE(*pmd, pmt);
	}

	/* CONSULT PGTABLE FOR VMA */
	pgd = pgd_offset(vma->vm_mm, vma->vm_start);
	if (pgd_none(*pgd))
		return -EBUSY; /* FORCE FAILED */

	orig_p4d = p4d_offset(pgd, vma->vm_start);
	if (!p4d_none(*orig_p4d))
		return -EBUSY; /* FORCE FAILED */

	/* FORCE POPULATE SPECIAL P4D */
	WRITE_ONCE(*orig_p4d, p4t);

	return 0;
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp,
                unsigned long uaddr, unsigned long len,
                unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
			len + ALIGN_512G, 0, flags);
	/* Aligned on 512G */
	align_addr = round_up(align_addr, ALIGN_512G);

	return align_addr;
}

static int BiscuitOS_open(struct inode *inode, struct file *filp)
{
	/* MONITOR PGTABLE RELEASE */
	notifier.ops = &BiscuitOS_mn_ops;
	mmu_notifier_register(&notifier, current->mm);

	return 0;
}

static int BiscuitOS_release(struct inode *inode, struct file *filp)
{
	mmu_notifier_unregister(&notifier, current->mm);

	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner             = THIS_MODULE,
	.open              = BiscuitOS_open,	
	.mmap              = BiscuitOS_mmap,
	.release           = BiscuitOS_release,
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

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
