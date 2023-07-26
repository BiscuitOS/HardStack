// SPDX-License-Identifier: GPL-2.0
/*
 * PUD(Page Upper Directory)
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

static int BiscuitOS_pud_entry(pud_t *pud, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long PMD_Table, orign_pud;
	pud_t puent = *pud;
	pgprot_t prot; 

	/* Check PUD Entry Present */
	if (pud_present(puent))
		printk("PUD Entry is Present.\n");

	/* Check PUD Entry is free */
	if (pud_none(puent))
		printk("PUD Entry is free.\n");

	/* Check PUD Entry is Bad value */
	if (pud_bad(puent))
		printk("PUD Entry is bad value\n");

	/* Check PUD is PGtable or Large Page */
	if (pud_large(puent))
		printk("PUD is 1G HugePage.\n");

	/* CHeck PUD is writable */
	if (pud_write(puent))
		printk("PUD is writable\n");

	/* Check PUD access permitted */
	if (pud_access_permitted(puent, true))
		printk("PUD is write permission.\n");

	/* Check PUD is young */
	if (pud_young(puent)) {
		printk("PUD is access later\n");
		puent = pud_mkold(puent);
	}

	/* Check PUD Entry Access */
	if (pud_flags(puent) & _PAGE_ACCESSED)
		printk("PUD Entry has Accessed.\n");
	if (pud_dirty(puent)) {
		printk("PUD Entry is Dirty.\n");
		puent = pud_mkclean(puent);
	}

	/* Check PUD Entry Attribute */
	if (pud_flags(puent) & _PAGE_RW)
		printk("PUD Entry is RW.\n");
	if (pud_flags(puent) & _PAGE_USER)
		printk("PUD Entry is Userspace mapping.\n");
	
	/* PUD Table Base Physical Address */
	PMD_Table = pud_pfn(puent) << PAGE_SHIFT;

	/* __pud: build PUD Entry */
	orign_pud = pud_val(*pud);
	puent = __pud(orign_pud);

	/* pgport */
	prot = pud_pgprot(puent);

	printk("PMD PageTable Attribute: %#lx\n", pgprot_val(prot));
	printk("PUD Entry Value %#lx\n", pud_val(puent));
	printk("PMD Table Physical Address %#lx\n", PMD_Table);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pud_entry	= BiscuitOS_pud_entry,
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
