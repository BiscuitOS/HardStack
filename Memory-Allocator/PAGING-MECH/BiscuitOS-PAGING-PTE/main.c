// SPDX-License-Identifier: GPL-2.0
/*
 * PTE(Page Table)
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

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long PhysPage, orign_pte;
	pte_t ptent = *pte;
	pgprot_t prot; 

	/* Check PTE Entry Present */
	if (pte_present(ptent))
		printk("PTE Entry is Present.\n");

	/* Check PTE Entry is free */
	if (pte_none(ptent))
		printk("PTE Entry is free.\n");

	/* CHeck PTE is writable */
	if (pte_write(ptent))
		printk("PTE is writable\n");

	/* Check PTE access permitted */
	if (pte_access_permitted(ptent, true))
		printk("PTE is write permission.\n");

	/* Check PTE is young */
	if (pte_young(ptent)) {
		printk("PTE is access later\n");
		ptent = pte_mkold(ptent);
	}

	/* Check PTE Entry Access */
	if (pte_flags(ptent) & _PAGE_ACCESSED)
		printk("PTE Entry has Accessed.\n");
	if (pte_dirty(ptent)) {
		printk("PTE Entry is Dirty.\n");
		ptent = pte_mkclean(ptent);
	}

	/* Check PTE Entry Attribute */
	if (pte_flags(ptent) & _PAGE_RW)
		printk("PTE Entry is RW.\n");
	if (pte_flags(ptent) & _PAGE_USER)
		printk("PTE Entry is Userspace mapping.\n");
	
	/* PTE Table Base Physical Address */
	PhysPage = pte_pfn(ptent) << PAGE_SHIFT;

	/* __pte: build PTE Entry */
	orign_pte = pte_val(*pte);
	ptent = __pte(orign_pte);

	/* pgport */
	prot = pte_pgprot(ptent);

	printk("Physical Page Attribute: %#lx\n", pgprot_val(prot));
	printk("PTE Entry Value %#lx\n", pte_val(ptent));
	printk("Physical Page Physical Address %#lx\n", PhysPage);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pte_entry	= BiscuitOS_pte_entry,
};

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	struct vm_area_struct *vma = find_vma(current->mm, arg);

	if (!vma)
		return -EINVAL;

	switch (ioctl) {
	case BS_WALK_PT:
		walk_page_range(vma->vm_mm, arg, arg + PAGE_SIZE,
				&BiscuitOS_pwalk_ops, NULL);
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
