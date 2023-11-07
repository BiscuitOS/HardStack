// SPDX-License-Identifier: GPL-2.0
/*
 * LOOKUP-ADDRESS: RSVDMEM
 *
 *    CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2020.10.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/io.h>

#define RSVDMEM_BASE	0x10000000
#define RSVDMEM_SIZE	0x1000

static int __init BiscuitOS_init(void)
{
	void *vaddr;
	pte_t *pte;
	int level;
	
	/* REMAPPING RSVDMEM */
	vaddr = memremap(RSVDMEM_BASE, RSVDMEM_SIZE, MEMREMAP_WB);
	if (!vaddr)
		return -ENOMEM;

	/* LOOKUP PTE */
	pte = lookup_address((unsigned long)vaddr, &level);
	printk("PADDR: %#lx\nVADDR: %#lx\nPTE:   %#lx LEVEL %d\n",
			(unsigned long)RSVDMEM_BASE, (unsigned long)vaddr,
			pte_val(*pte), level);

	/* RECLAIM */
	memunmap(vaddr);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PAGING Project");
