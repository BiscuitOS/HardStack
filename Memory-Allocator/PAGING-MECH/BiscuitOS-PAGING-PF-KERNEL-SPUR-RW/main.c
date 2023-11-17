// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel: Spurious #PF
 *
 *   CMDLINE ADD "memmap=4K$0x10000000"
 *
 * (C) 2023.11.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <asm/tlbflush.h>

#define RSVDMEM_BASE		0x10000000

static int __init BiscuitOS_init(void)
{
	pte_t *pte, ptent;
	void *mem;
	int level;	

	/* MAPPING RSVDMEM MEMORY */
	mem = memremap(RSVDMEM_BASE, PAGE_SIZE, MEMREMAP_WB);
	if (!mem)
		return -ENOMEM;

	/* CONSULT PTE */
	pte = lookup_address((unsigned long)mem, &level);
	if (level != 1)
		return -EINVAL; /* Only Handle PTE */

	/* SET WRITE-PROTECTION */
	ptent = pte_wrprotect(*pte);
	set_pte_at(&init_mm, (unsigned long)mem, pte, ptent);
	if (!pte_write(*pte))
		printk("SPUR-WP: %#lx\n", (unsigned long)mem);
	
	/* LOAD TLB */
	flush_tlb_kernel_range((unsigned long)mem, (unsigned long)mem + PAGE_SIZE);
	printk("SPUR-RDLOAD: %#lx => %c\n", (unsigned long)mem, *(char *)mem);

	/* CLEAR WP */
	ptent = pte_mkwrite(*pte);
	set_pte_at(&init_mm, (unsigned long)mem, pte, ptent);
	if (pte_write(*pte))
		printk("SPUR-WR: %#lx\n", (unsigned long)mem);

	/* WRITE-OPS, Trigger #SPUR-PF */
	*(char *)mem = 'D';

	/* RECLAIM */
	memunmap(mem);
	
	return 0;
}
__initcall(BiscuitOS_init);
