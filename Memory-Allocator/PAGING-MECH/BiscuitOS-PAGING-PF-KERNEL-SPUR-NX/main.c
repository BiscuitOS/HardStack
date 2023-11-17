// SPDX-License-Identifier: GPL-2.0
/*
 * PageFault on Kernel: Spurious #PF
 *
 * (C) 2023.11.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <asm/tlbflush.h>

/* Machine Code Running on X86 Kernel */
char MACH_CODE[] = { 0xc3, 0x00, 0x00 /* retq */ };

typedef void (*spur_func_t)(void);

static int __init BiscuitOS_init(void)
{
	struct page *page;
	pte_t *pte, ptent;
	spur_func_t func;
	void *mem;
	int level;	

	/* ALLOC PHYSICAL MEMORY */
	page = alloc_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	/* MAPPING NX MEMORY */
	mem = vmap(&page, 1, VM_MAP, PAGE_KERNEL);
	if (!mem)
		return -ENOMEM;

	/* INSTALL CODE */
	memset(mem, 0x00, PAGE_SIZE);
	memcpy(mem, (void *)MACH_CODE, 3);
	func = (spur_func_t)mem;

	/* CONSULT PTE */
	pte = lookup_address((unsigned long)mem, &level);
	if (level != 1)
		return -EINVAL; /* Only Handle PTE */

	/* CLEAR EXEC */
	ptent = pte_set_flags(*pte, _PAGE_NX);
	set_pte_at(&init_mm, (unsigned long)mem, pte, ptent);
	if (!pte_exec(*pte))
		printk("SPUR-NX: %#lx => %#lx\n",
				(unsigned long)mem, pte_val(*pte));
	
	/* LOAD TLB */
	flush_tlb_kernel_range((unsigned long)mem,
			       (unsigned long)mem + PAGE_SIZE);
	printk("EXEC-CODE: %#lx\n", *(unsigned long *)mem);

	/* SET EXEC */
	ptent = pte_clear_flags(*pte, _PAGE_NX);
	set_pte_at(&init_mm, (unsigned long)mem, pte, ptent);
	if (pte_exec(*pte))
		printk("SPUR-EXEC: %#lx\n", pte_val(*pte));

	/* EXEC-OPS, Trigger #SPUR-PF */
	func();

	/* RECLAIM */
	memunmap(mem);
	__free_page(page);
	
	return 0;
}
__initcall(BiscuitOS_init);
