// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with RSVD-MEM
 *  RSVDMEM on CMDLINE: 4K$0x2000000
 *
 * (C) 2023.07.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <asm/pgtable.h>
#include <linux/io.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)
#define RSVDMEM_BASE		0x2000000

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
			unsigned long next, struct mm_walk *walk)
{
	if (pte_none(*pte))
		return 0;

	printk("Virtual Addr: %#lx\n", addr);
	printk("PageTB PTE:   %#lx\n", pte_val(*pte));
	printk("RSVDMEM Phys: %#lx\n", pte_pfn(*pte) << PAGE_SHIFT);

	return 0;
}

static int BiscuitOS_pte_hole(unsigned long addr, unsigned long next,
				int depth, struct mm_walk *walk)
{
	if (depth == -1)
		printk("VMA contain VM_PFNMAP, Stop walk page.\n");

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pte_entry	= BiscuitOS_pte_entry,
	.pte_hole	= BiscuitOS_pte_hole,
};

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct vm_area_struct *vma;

	mmap_write_lock_killable(current->mm);
	vma = find_vma(current->mm, arg);
	if (!vma) {
		mmap_write_unlock(current->mm);
		return -EINVAL;
	}

	switch (ioctl) {
	case BS_WALK_PT:
		bs_debug_enable();
		walk_page_range(vma->vm_mm, arg, arg + PAGE_SIZE,
				&BiscuitOS_pwalk_ops, NULL);
		bs_debug_disable();
		break;
	}
	mmap_write_unlock(current->mm);
	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
			RSVDMEM_BASE >> PAGE_SHIFT,
			PAGE_SIZE, vma->vm_page_prot);
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.mmap		= BiscuitOS_mmap,
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
