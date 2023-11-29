// SPDX-License-Identifier: GPL-2.0
/*
 * REVERSE MAPPING: FILE MAPPING
 *
 * (C) 2023.11.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/rmap.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define DEV_NAME		"BiscuitOS-RMAP"
#define BISCUITOS_IO		0xBD
#define CONSULT_RMAP		_IO(BISCUITOS_IO, 0x00)

static int consult_rmap(pte_t *pte, unsigned long addr, void *data)
{
	pgoff_t pgoff_start, pgoff_end;
	struct vm_area_struct *vma;
	struct page *page;

	/* PREPARE PAGE */
	page = pfn_to_page(pte_pfn(*pte));

	/* CONSULT VMA */
	pgoff_start = page->index;
	pgoff_end   = page->index + 1;
	vma_interval_tree_foreach(vma, &page->mapping->i_mmap, 
						pgoff_start, pgoff_end)
		printk("VMA: PID-%d\n", vma->vm_mm->owner->pid);

	return 0;
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case CONSULT_RMAP:
		mmap_write_lock_killable(current->mm);
		apply_to_existing_page_range(current->mm, arg,
					PAGE_SIZE, consult_rmap, NULL);
		mmap_write_unlock(current->mm);
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
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int __init BiscuitOS_init(void)
{
	misc_register(&BiscuitOS_drv);
	return 0;
}
__initcall(BiscuitOS_init);
