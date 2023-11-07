// SPDX-License-Identifier: GPL-2.0
/*
 * Linear Mapping: Consult PFN Range
 *
 * (C) 2023.10.30 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/page_types.h>

#define PHYS_ADDR	0x10000000UL

static int __init BiscuitOS_init(void)
{
	int i;

	/* Consult */
	if (pfn_range_is_mapped(PHYS_PFN(PHYS_ADDR), 
				PHYS_PFN(PHYS_ADDR + PAGE_SIZE)))
		printk("LM %#lx - %#lx has mapped.\n",
				PHYS_ADDR, PHYS_ADDR + PAGE_SIZE);
	/* Traverse */
	printk("Linear Mapping Phyiscal Area:\n");
	for (i = 0; i < nr_pfn_mapped; i++)
		printk(" RANGE%d: %#llx - %#llx\n", i,
			pfn_mapped[i].start, pfn_mapped[i].end);

	return 0;
}
__initcall(BiscuitOS_init);
