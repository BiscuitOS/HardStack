// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with Modify PageTable
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
#define BS_MODIFY_PT		_IO(BISCUITOS_IO, 0x00)

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
                         unsigned long next, struct mm_walk *walk)
{
	pte_t ptent;

	if (pte_none(*pte))
		return 0;

	/* Modify PageTable */
	ptent = ptep_get_and_clear(walk->mm, addr, pte);
	/* Read-Only */
	ptent = pte_wrprotect(ptent);
	/* Set PT */
	set_pte(pte, ptent);
	walk->vma->vm_flags &= ~VM_WRITE;

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
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
	case BS_MODIFY_PT:
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
