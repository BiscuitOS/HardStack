// SPDX-License-Identifier: GPL-2.0
/*
 * P4D(Page 4th Directory)
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

static int BiscuitOS_p4d_entry(p4d_t *p4d, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	unsigned long PUD_Table, orign_p4d;
	p4d_t p4ent = *p4d;
	pgprot_t prot; 

	/* Check P4D Entry Present */
	if (p4d_present(p4ent))
		printk("P4D Entry is Present.\n");

	/* Check P4D Entry is free */
	if (p4d_none(p4ent))
		printk("P4D Entry is free.\n");

	/* Check P4D Entry is Bad value */
	if (p4d_bad(p4ent))
		printk("P4D Entry is bad value\n");

	/* Check P4D Entry Access */
	if (p4d_flags(p4ent) & _PAGE_ACCESSED)
		printk("P4D Entry has Accessed.\n");
	if (p4d_flags(p4ent) & _PAGE_DIRTY)
		printk("P4D Entry is Dirty.\n");

	/* Check P4D Entry Attribute */
	if (p4d_flags(p4ent) & _PAGE_RW)
		printk("P4D Entry is RW.\n");
	if (p4d_flags(p4ent) & _PAGE_USER)
		printk("P4D Entry is Userspace mapping.\n");
	
	/* P4D Table Base Physical Address */
	PUD_Table = p4d_pfn(p4ent) << PAGE_SHIFT;

	/* __p4d: build P4D Entry */
	orign_p4d = p4d_val(*p4d);
	p4ent = __p4d(orign_p4d);

	/* pgport */
	prot = p4d_pgprot(p4ent);

	printk("PUD PageTable Attribute: %#lx\n", pgprot_val(prot));
	printk("P4D Entry Value %#lx\n", p4d_val(p4ent));
	printk("PUD Table Physical Address %#lx\n", PUD_Table);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.p4d_entry	= BiscuitOS_p4d_entry,
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
