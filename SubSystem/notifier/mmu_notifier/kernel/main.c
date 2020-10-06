/*
 * MMU notifier on BiscuitOS
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/* MMU notifier */
#include <linux/mmu_notifier.h>
/* Current */
#include <linux/sched.h>

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS"

static struct mmu_notifier BiscuitOS_notifier;
static struct mmu_notifier_range BiscuitOS_range;

static void BiscuitOS_mmu_release(struct mmu_notifier *mn,
                                                struct mm_struct *mm)
{
	printk("BiscuitOS notifier: release\n");
}

static int BiscuitOS_mmu_clear_flush_young(struct mmu_notifier *mn,
                struct mm_struct *mm, unsigned long start, unsigned long end)
{
	printk("BiscuitOS notifier: clear_flush_young\n");
	return 0;
}

static int BiscuitOS_mmu_clear_young(struct mmu_notifier *mn,
                struct mm_struct *mm, unsigned long start, unsigned long end)
{
	printk("BiscuitOS notifier: clear_young\n");
	return 0;
}

static int BiscuitOS_mmu_test_young(struct mmu_notifier *mn,
                        struct mm_struct *mm, unsigned long address)
{
	printk("BiscuitOS notifier: test_young\n");
	return 0;
}

static void BiscuitOS_mmu_change_pte(struct mmu_notifier *mn,
                struct mm_struct *mm, unsigned long address, pte_t pte)
{
	printk("BiscuitOS notifier: change_pte\n");
}

static int BiscuitOS_mmu_invalidate_range_start(struct mmu_notifier *mn,
                                const struct mmu_notifier_range *range)
{
	printk("BiscuitOS notifier: invalidate_range_start.\n");
	return 0;
}

static void BiscuitOS_mmu_invalidate_range_end(struct mmu_notifier *mn,
                                const struct mmu_notifier_range *range)
{
	printk("BiscuitOS notifier: invalidate_range_end.\n");
}

static void BiscuitOS_mmu_invalidate_range(struct mmu_notifier *mn,
                struct mm_struct *mm, unsigned long start, unsigned long end)
{
	printk("BiscuitOS notifier: invalidate_range.\n");
}

static const struct mmu_notifier_ops BiscuitOS_mmu_notifer_ops = {
	.release     = BiscuitOS_mmu_release,
	.clear_young = BiscuitOS_mmu_clear_young,
	.test_young  = BiscuitOS_mmu_test_young,
	.change_pte  = BiscuitOS_mmu_change_pte,
	.clear_flush_young = BiscuitOS_mmu_clear_flush_young,
	.invalidate_range  = BiscuitOS_mmu_invalidate_range,
	.invalidate_range_start = BiscuitOS_mmu_invalidate_range_start,
	.invalidate_range_end   = BiscuitOS_mmu_invalidate_range_end,
};

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct mm_struct *mm = filp->private_data;
	pte_t pte;

	/* Trigger invalidate range [range, start, end] */
	mmu_notifier_range_init(&BiscuitOS_range, mm, 
		vma->vm_start & PAGE_MASK, vma->vm_end & PAGE_MASK);
	mmu_notifier_invalidate_range_start(&BiscuitOS_range);
	mmu_notifier_invalidate_range_end(&BiscuitOS_range);

	/* Trigger clear_flush_young */
	mmu_notifier_clear_flush_young(mm, vma->vm_start, vma->vm_end);

	/* Trigger clear_young */
	mmu_notifier_clear_young(mm, vma->vm_start, vma->vm_end);

	/* Trigger test_young */
	mmu_notifier_test_young(mm, vma->vm_start);

	/* Trigger change pte */
	mmu_notifier_change_pte(mm, vma->vm_start, pte);

	/* Trigger realease */
	mmu_notifier_release(mm);

	return 0;
}

static int BiscuitOS_open(struct inode *inode, struct file *file)
{
	struct mm_struct *mm = get_task_mm(current);

	file->private_data = mm;
	/* mmu notifier initialize */
	BiscuitOS_notifier.ops = &BiscuitOS_mmu_notifer_ops;
	/* mmu notifier register */
	mmu_notifier_register(&BiscuitOS_notifier, mm);

	return 0;
}

static int BiscuitOS_release(struct inode *inode, struct file *file)
{
	mmu_notifier_unregister(&BiscuitOS_notifier, current->mm);
	return 0;
}

/* file operations */
static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.open		= BiscuitOS_open,
	.mmap		= BiscuitOS_mmap,
	.release	= BiscuitOS_release,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

/* Module initialize entry */
static void __init BiscuitOS_init(void)
{
	/* Register Misc device */
	misc_register(&BiscuitOS_drv);

	printk("Hello modules on BiscuitOS\n");
}

device_initcall(BiscuitOS_init);
