// SPDX-License-Identifier: GPL-2.0-only
/*
 * PFNMAP: CUSTOMIZE MAPPED 1Gig
 *
 *   Add "memmap=2M$4G" into CMDLINE
 *
 * (C) 2023.11.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pfn_t.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PFNMAP"
#define PFN_PHYSADDR		0x100000000 /* 128MiB */

static vm_fault_t vm_huge_fault(struct vm_fault *vmf, 
				enum page_entry_size pe_size)
{
	pfn_t pfn = phys_to_pfn_t(PFN_PHYSADDR, PFN_DEV);

	return vmf_insert_pfn_pud(vmf, pfn, true);
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
	/* FPNMAP */
	vma->vm_flags |= VM_PFNMAP;

	return 0;
}

static unsigned long BiscuitOS_get_unmapped_area(struct file *filp,
                unsigned long uaddr, unsigned long len,
                unsigned long pgoff, unsigned long flags)
{
	unsigned long align_addr;

	align_addr = current->mm->get_unmapped_area(NULL, 0,
			len + PUD_SIZE, 0, flags);
	/* Aligned on 1Gig */
	align_addr = round_up(align_addr, PUD_SIZE);

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

static void __exit BiscuitOS_exit(void)
{
	misc_deregister(&BiscuitOS_drv);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
