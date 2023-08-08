// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with THP memory
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
#include <linux/hugetlb.h>

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
			unsigned long next, struct mm_walk *walk)
{
	if (pmd_none(*pmd))
		return 0;

	printk("Virtual Addr: %#lx\n", addr);
	printk("PageTB PMD:   %#lx\n", pmd_val(*pmd));
	printk("2MiB Phys:    %#lx\n", pmd_pfn(*pmd) << PAGE_SHIFT);

	pmd_clear(pmd); /* Force Clear */
	walk->action = ACTION_AGAIN; /* Goto pte_hole depth==3 */

	return 0;
}

static int BiscuitOS_pte_hole(unsigned long addr, unsigned long next,
			int depth, struct mm_walk *walk)
{
	/* PMD Entry Empty */
	if (depth == 3)
		printk("%#lx PMD Entry is empty.\n", addr);
	return 0; /* 0 for continue next PMD Entry.
		   * !0 for break Pagewalk */
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pmd_entry	= BiscuitOS_pmd_entry,
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
		walk_page_range(vma->vm_mm, arg, arg + PAGE_SIZE,
				&BiscuitOS_pwalk_ops, NULL);
		break;
	}
	mmap_write_unlock(current->mm);
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
	return misc_register(&BiscuitOS_drv);
}
device_initcall(BiscuitOS_init);
