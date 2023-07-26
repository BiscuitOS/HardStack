// SPDX-License-Identifier: GPL-2.0
/*
 * PGD(Page Global Directory)
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

#define SPECIAL_DEV_NAME	"BiscuitOS-PageTable"
#define BISCUITOS_IO		0xBD
#define BS_WALK_PT		_IO(BISCUITOS_IO, 0x00)

static int BiscuitOS_pgd_entry(pgd_t *pgd, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long P4D_Table, orign_pgd;
	pgd_t pgent = *pgd;

	/* Check PGD Entry Present */
	if (pgd_present(pgent))
		printk("PGD Entry is Present.\n");

	/* Check PGD Entry is free */
	if (pgd_none(pgent))
		printk("PGD Entry is free.\n");

	/* Check PGD Entry is Bad value */
	if (pgd_bad(pgent))
		printk("PGD Entry is bad value\n");

	/* Check PGD Entry Access */
	if (pgd_flags(pgent) & _PAGE_ACCESSED)
		printk("PGD Entry has Accessed.\n");
	if (pgd_flags(pgent) & _PAGE_DIRTY)
		printk("PGD Entry is Dirty.\n");

	/* Check PGD Entry Attribute */
	if (pgd_flags(pgent) & _PAGE_RW)
		printk("PGD Entry is RW.\n");
	if (pgd_flags(pgent) & _PAGE_USER)
		printk("PGD Entry is Userspace mapping.\n");
	
	/* P4D Table Base Physical Address */
	P4D_Table = pgd_pfn(pgent) << PAGE_SHIFT;

	/* __pgd: build PGD Entry */
	orign_pgd = pgd_val(*pgd);
	pgent = __pgd(orign_pgd);

	printk("PGD Entry Value %#lx\n", pgd_val(pgent));
	printk("P4D Table Physical Address %#lx\n", P4D_Table);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pgd_entry	= BiscuitOS_pgd_entry,
};

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct vm_area_struct *vma = find_vma(current->mm, arg);

	if (!vma)
		return -EINVAL;

	switch (ioctl) {
	case BS_WALK_PT:
		walk_page_vma(vma, &BiscuitOS_pwalk_ops, (void *)arg);
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
	return misc_register(&BiscuitOS_drv);
}
device_initcall(BiscuitOS_init);
