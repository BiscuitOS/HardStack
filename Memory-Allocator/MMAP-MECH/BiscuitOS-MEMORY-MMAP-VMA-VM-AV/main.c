// SPDX-License-Identifier: GPL-2.0
/*
 * MMAP: VMA AV and AVC
 *
 * (C) 2023.12.27 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/rmap.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define DEV_NAME		"BiscuitOS-VMA"
#define BISCUITOS_IO		0xBD
#define CONSULT_RMAP		_IO(BISCUITOS_IO, 0x00)

static int consult_rmap(pte_t *pte, unsigned long addr, void *data)
{
	struct vm_area_struct *vma = find_vma(current->mm, addr);
	struct anon_vma_chain *pavc, *savc;
	pgoff_t pgoff_start, pgoff_end;
	struct anon_vma *av;
	struct page *page;

	if (!vma)
		return -ENOMEM;

	/* PREPARE PAGE */
	page = pfn_to_page(pte_pfn(*pte));

	/* CONSULT CHILD */
	pgoff_start = page->index;
	pgoff_end   = pgoff_start + 1;
	/* AV */
	av = (struct anon_vma *)((void *)page->mapping -
						PAGE_MAPPING_ANON);
	/* TRAVER RMAP */
	anon_vma_interval_tree_foreach(savc, &av->rb_root, 
						pgoff_start, pgoff_end)
		printk("RMAP-VMA: PID-%d\n", savc->vma->vm_mm->owner->pid);

	/* CONSULT FATHER */
	list_for_each_entry(pavc, &vma->anon_vma_chain, same_vma)
		printk("FATHER-AV: %#lx\n", (unsigned long)pavc->anon_vma);

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
