// SPDX-License-Identifier: GPL-2.0
/*
 * PMD(Page Middle Directory)
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

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long PTE_Table, orign_pmd;
	pmd_t pment = *pmd;
	pgprot_t prot; 

	/* Check PMD Entry Present */
	if (pmd_present(pment))
		printk("PMD Entry is Present.\n");

	/* Check PMD Entry is free */
	if (pmd_none(pment))
		printk("PMD Entry is free.\n");

	/* Check PMD Entry is Bad value */
	if (pmd_bad(pment))
		printk("PMD Entry is bad value\n");

	/* Check PMD is PGtable or Large Page */
	if (pmd_large(pment))
		printk("PMD is 1G HugePage.\n");

	/* CHeck PMD is writable */
	if (pmd_write(pment))
		printk("PMD is writable\n");

	/* Check PMD access permitted */
	if (pmd_access_permitted(pment, true))
		printk("PMD is write permission.\n");

	/* Check PMD is young */
	if (pmd_young(pment)) {
		printk("PMD is access later\n");
		pment = pmd_mkold(pment);
	}

	/* Check PMD Entry Access */
	if (pmd_flags(pment) & _PAGE_ACCESSED)
		printk("PMD Entry has Accessed.\n");
	if (pmd_dirty(pment)) {
		printk("PMD Entry is Dirty.\n");
		pment = pmd_mkclean(pment);
	}

	/* Check PMD Entry Attribute */
	if (pmd_flags(pment) & _PAGE_RW)
		printk("PMD Entry is RW.\n");
	if (pmd_flags(pment) & _PAGE_USER)
		printk("PMD Entry is Userspace mapping.\n");
	
	/* PTE Table Base Physical Address */
	PTE_Table = pmd_pfn(pment) << PAGE_SHIFT;

	/* __pmd: build PMD Entry */
	orign_pmd = pmd_val(*pmd);
	pment = __pmd(orign_pmd);

	/* pgport */
	prot = pmd_pgprot(pment);

	printk("PTE PageTable Attribute: %#lx\n", pgprot_val(prot));
	printk("PMD Entry Value %#lx\n", pmd_val(pment));
	printk("PTE Table Physical Address %#lx\n", PTE_Table);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pmd_entry	= BiscuitOS_pmd_entry,
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
