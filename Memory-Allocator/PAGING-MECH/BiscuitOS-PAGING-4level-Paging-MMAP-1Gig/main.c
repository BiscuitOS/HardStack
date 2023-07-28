// SPDX-License-Identifier: GPL-2.0
/*
 * 4 level Paging Mapping 4MiB
 *  CMDLINE: memmap=1G$0x10000000
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
#ifdef __i386__
#error "Must Running on Intel X86"
#endif

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define HPAGE_SIZE_1G		(1024 * 1024 * 1024)
#define RSVD_MEM		0x100000000

static int BiscuitOS_p4d_entry(p4d_t *p4d, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long pfn = RSVD_MEM >> PAGE_SHIFT;
	struct vm_area_struct *vma = walk->vma;
	spinlock_t *ptl;
	pud_t *pud;

	/* Check PUD Entry is Empty, Address must alignment 1Gig */
	pud = pud_offset(p4d, addr);
	if (!pud_none(*pud))
		return -EINVAL;

	ptl = pud_lock(vma->vm_mm, pud);
	mm_inc_nr_puds(vma->vm_mm);
	smp_wmb();
	/* Setup Pagetable for 4MiB */
	set_pud_at(vma->vm_mm, addr, pud,
		pud_mkhuge(pfn_pud(pfn, vma->vm_page_prot)));
	spin_unlock(ptl);
	printk("Build 1Gig Page Mapping.\n");

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.p4d_entry	= BiscuitOS_p4d_entry,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	walk_page_vma(vma, &BiscuitOS_pwalk_ops, NULL);

	vma->vm_flags |= VM_PFNMAP;
	return 0;
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp,
		unsigned long uaddr, unsigned long len,
		unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
			len + HPAGE_SIZE_1G, 0, flags);
	/* Aligned on 4MiB */
	align_addr = round_up(align_addr, HPAGE_SIZE_1G);

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
