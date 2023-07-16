// SPDX-License-Identifier: GPL-2.0
/*
 * TLB FLUSH
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

#define SPECIAL_DEV_NAME	"BiscuitOS"
#define RSVDMEM_BASE		0x10000000
#define RSVDMEM_SIZE		0x1000

#define BISCUITOS_IO		0xBD
#define BS_FLUSH_TLB		_IO(BISCUITOS_IO, 0x00)

/* FLUSH TLB */
static int BiscuitOS_flush_tlb(unsigned long addr)
{
	struct mmu_gather tlb;
	struct vm_area_struct *vma;

	vma = find_vma(current->mm, addr);
	if (!vma)
		return -ENOMEM;

	tlb_gather_mmu(&tlb, current->mm);
	tlb_start_vma(&tlb, vma);
	/* Insert Flush Range */
	tlb_flush_pte_range(&tlb, addr, PAGE_SIZE);	
	tlb_end_vma(&tlb, vma); /* FLUSH TLB One */
	tlb_finish_mmu(&tlb);
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

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.mmap		= BiscuitOS_mmap,
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
