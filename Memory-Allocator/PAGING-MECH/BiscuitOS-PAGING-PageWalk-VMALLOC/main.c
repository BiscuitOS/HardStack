// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with VMALLOC
 *
 * (C) 2023.07.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pagewalk.h>
#include <linux/vmalloc.h>

static int BiscuitOS_pte_entry(pte_t *pte, unsigned long addr,
			unsigned long next, struct mm_walk *walk)
{
	if (pte_none(*pte))
		return 0;

	printk("Virtual Addr: %#lx\n", addr);
	printk("PageTB PTE:   %#lx\n", pte_val(*pte));
	printk("Phys:         %#lx\n", pte_pfn(*pte) << PAGE_SHIFT);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pte_entry	= BiscuitOS_pte_entry,
};

static int __init BiscuitOS_init(void)
{
	void *base;

	base = vmalloc(HPAGE_SIZE);
	if (!base) {
		printk("ERROR: No Free Memory.\n");
		return -ENOMEM;
	}

	mmap_write_lock_killable(&init_mm);

	walk_page_range_novma(&init_mm, (unsigned long)base,
			(unsigned long)base + PAGE_SIZE,
			&BiscuitOS_pwalk_ops, init_mm.pgd, NULL);

	mmap_write_unlock(&init_mm);

	vfree(base);

	return 0;
}
device_initcall(BiscuitOS_init);
