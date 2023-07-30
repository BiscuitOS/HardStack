// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk Mechanism
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

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	/* Check PTE Entry Present */
	if (pte_present(*pte))
		printk("PTE Entry is Present %#lx.\n", pte_val(*pte));

	return 0;
}

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	/* Check PMD Entry Present */
	if (pmd_present(*pmd))
		printk("PMD Entry is Present %#lx.\n", pmd_val(*pmd));

	return 0;
}

static int BiscuitOS_pud_entry(pud_t *pud, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	/* Check PUD Entry Present */
	if (pud_present(*pud))
		printk("PUD Entry is Present %#lx.\n", pud_val(*pud));

	return 0;
}

static int BiscuitOS_p4d_entry(p4d_t *p4d, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	/* Check P4D Entry Present */
	if (p4d_present(*p4d))
		printk("P4D Entry is Present %#lx.\n", p4d_val(*p4d));

	return 0;
}

static int BiscuitOS_pgd_entry(pgd_t *pgd, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	/* Check PGD Entry Present */
	if (pgd_present(*pgd))
		printk("PGD Entry is Present %#lx.\n", pgd_val(*pgd));

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pgd_entry	= BiscuitOS_pgd_entry,
	.p4d_entry	= BiscuitOS_p4d_entry,
	.pud_entry	= BiscuitOS_pud_entry,
	.pmd_entry	= BiscuitOS_pmd_entry,
	.pte_entry	= BiscuitOS_pte_entry,
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
