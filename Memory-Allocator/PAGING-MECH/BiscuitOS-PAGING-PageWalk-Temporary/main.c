// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with Temporary Mapping
 *
 * (C) 2023.08.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <linux/highmem.h>

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
			unsigned long next, struct mm_walk *walk)
{
	if (pmd_none(*pmd))
		return 0;

	printk("Virtual Addr: %#lx\n", addr);
	printk("PageTB PMD:   %#lx\n", pmd_val(*pmd));
	printk("Phys:         %#lx\n", pmd_pfn(*pmd) << PAGE_SHIFT);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pmd_entry	= BiscuitOS_pmd_entry,
};

static int __init BiscuitOS_init(void)
{
	unsigned long base;
	struct page *page;

	page = alloc_page(GFP_KERNEL);
	if (!page) {
		printk("System Error: No free Memory.\n");
		return -ENOMEM;
	}
	/* Temporary mapping */
	base = (unsigned long)kmap(page);
	printk("Temporary VA: %#lx PFN: %#lx\n", base, page_to_pfn(page));

	mmap_write_lock_killable(&init_mm);

	walk_page_range_novma(&init_mm, base, base + PAGE_SIZE,
			&BiscuitOS_pwalk_ops, init_mm.pgd, NULL);

	mmap_write_unlock(&init_mm);

	kunmap(page);
	__free_page(page);

	return 0;
}
device_initcall(BiscuitOS_init);
