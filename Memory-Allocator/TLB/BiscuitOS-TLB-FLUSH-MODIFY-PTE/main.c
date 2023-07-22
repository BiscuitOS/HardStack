// SPDX-License-Identifier: GPL-2.0
/*
 * Modify PTE without FLUSH TLB
 *
 * (C) 2023.05.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <linux/pagewalk.h>

#define SPECIAL_DEV_NAME	"BiscuitOS"
#define BISCUITOS_IO		0xBD
#define BS_FLUSH_TLB		_IO(BISCUITOS_IO, 0x00)
#define BS_NOFLUSH_TLB		_IO(BISCUITOS_IO, 0x01)

static int BiscuitOS_flush(pte_t *pte, unsigned long addr, bool flush)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);

	if (!vma)
		return -EINVAL;

	if (flush) { /* FLUSH TLB */
		pte_t ptent = ptep_get_and_clear(vma->vm_mm, addr, pte);
		struct mmu_gather tlb;

		tlb_gather_mmu(&tlb, vma->vm_mm);
		tlb_start_vma(&tlb, vma);
		tlb_change_page_size(&tlb, PAGE_SIZE);

		vma->vm_flags &= ~VM_WRITE;
		ptent = pte_wrprotect(ptent);
		set_pte_at(vma->vm_mm, addr, pte, ptent);
		tlb_flush_pte_range(&tlb, addr, PAGE_SIZE);

		tlb_end_vma(&tlb, vma);
		tlb_finish_mmu(&tlb);
		printk("Change PTE With FLUSH TLB.\n");
	} else { /* Dont FLUSH TLB */
		pte_t ptent = ptep_get_and_clear(vma->vm_mm, addr, pte);

		vma->vm_flags &= ~VM_WRITE;
		ptent = pte_wrprotect(ptent);
		set_pte_at(vma->vm_mm, addr, pte, ptent);
		printk("Change PTE Without FLUSH TLB.\n");
	}

	return 0;
}

static int BiscuitOS_flush_tlb(unsigned long addr, bool flush)
{
	/* PageTable Must Present! */
	pgd_t *pgd = pgd_offset(current->mm, addr);
	p4d_t *p4d = p4d_offset(pgd, addr);
	pud_t *pud = pud_offset(p4d, addr);
	pmd_t *pmd = pmd_offset(pud, addr);
	pte_t *pte = pte_offset_map(pmd, addr);

	return BiscuitOS_flush(pte, addr, flush);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BS_FLUSH_TLB:
		BiscuitOS_flush_tlb(arg, true);
		break;
	case BS_NOFLUSH_TLB:
		BiscuitOS_flush_tlb(arg, false);
		break;	
	}
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
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
