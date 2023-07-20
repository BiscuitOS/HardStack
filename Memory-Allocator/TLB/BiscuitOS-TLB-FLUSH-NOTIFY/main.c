// SPDX-License-Identifier: GPL-2.0
/*
 * TLB FLUSH Nodifier
 *  - add into cmdline '4K$0x10000000'
 *  
 * (C) 2023.05.15 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <linux/mmu_notifier.h>

#define SPECIAL_DEV_NAME	"BiscuitOS"
#define RSVDMEM_BASE		0x10000000
#define RSVDMEM_SIZE		0x1000

#define BISCUITOS_IO		0xBD
#define BS_FLUSH_TLB		_IO(BISCUITOS_IO, 0x00)

static struct mmu_notifier BiscuitOS_notifier;
static struct mmu_notifier_range BiscuitOS_range;
static unsigned long invalidate_addr;

static void BiscuitOS_invalidate_range(struct mmu_notifier *mn,
		struct mm_struct *mm, unsigned long start, unsigned long end)
{
	if (start != invalidate_addr)
		return;
	printk("Invalidate Range %#lx - %#lx", start, end);
}

static int BiscuitOS_invalidate_range_start(struct mmu_notifier *mn,
				const struct mmu_notifier_range *range)
{
	if (range->start != invalidate_addr)
		return 0;
	printk("Invalidate Begin.\n");
	return 0;
}

void BiscuitOS_invalidate_range_end(struct mmu_notifier *mn,
				const struct mmu_notifier_range *range)
{
	if (range->start != invalidate_addr)
		return;
	printk("Invalidate End.\n");
}

static const struct mmu_notifier_ops BiscuitOS_notifier_ops = {
	.invalidate_range	= BiscuitOS_invalidate_range,
	.invalidate_range_start	= BiscuitOS_invalidate_range_start,
	.invalidate_range_end	= BiscuitOS_invalidate_range_end,
};

/* FLUSH TLB */
static int BiscuitOS_flush_tlb(unsigned long addr)
{
	struct mmu_gather tlb;
	struct vm_area_struct *vma;

	vma = find_vma(current->mm, addr);
	if (!vma)
		return -ENOMEM;

	invalidate_addr = addr;
	/* Notifier */
	mmu_notifier_range_init(&BiscuitOS_range, MMU_NOTIFY_UNMAP,
			0, vma, vma->vm_mm, addr & PAGE_MASK, 
			(addr + PAGE_SIZE) & PAGE_MASK);
	mmu_notifier_invalidate_range_start(&BiscuitOS_range);

	tlb_gather_mmu(&tlb, current->mm);
	tlb_start_vma(&tlb, vma);
	/* Insert Flush Range */
	tlb_flush_pte_range(&tlb, addr, PAGE_SIZE);	
	tlb_end_vma(&tlb, vma); /* FLUSH TLB One */
	tlb_finish_mmu(&tlb);

	mmu_notifier_invalidate_range_end(&BiscuitOS_range);
	mmu_notifier_synchronize();
	invalidate_addr = 0;
	printk("FLUSH TLB Finish.\n");
	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
			RSVDMEM_BASE >> PAGE_SHIFT,
			PAGE_SIZE, vma->vm_page_prot);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BS_FLUSH_TLB:
		BiscuitOS_flush_tlb(arg);
		break;
	}
	return 0;
}

static int BiscuitOS_open(struct inode *inode, struct file *file)
{
	struct mm_struct *mm = get_task_mm(current);

	/* MMU notifier initialize */
	BiscuitOS_notifier.ops = &BiscuitOS_notifier_ops;
	/* MMU notifier regiseter */
	mmu_notifier_register(&BiscuitOS_notifier, mm);

	return 0;
}

static int BiscuitOS_release(struct inode *inode, struct file *filp)
{
	mmu_notifier_unregister(&BiscuitOS_notifier, current->mm);
	return 0;
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.open		= BiscuitOS_open,
	.release	= BiscuitOS_release,
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
