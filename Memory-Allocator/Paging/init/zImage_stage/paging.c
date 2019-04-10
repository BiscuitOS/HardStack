/*
 * ARMv7 Paging 
 *
 * (C) 2019.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>

static int debug_zImage_page_table(void)
{
	unsigned long D0;
	unsigned long *pgd;
	unsigned long index;

	/* Allocate memory to pgd */
	pgd = (unsigned long *)kmalloc(10 * PAGE_SIZE, GFP_KERNEL);
	if (!pgd) {
		printk("No free Memory.\n");
		return 0;
	} else {
		/* write to main-memory not write-buffer */
		memset(pgd, 0, 10 * PAGE_SIZE);
	}

	__asm__ volatile ("mrc	p15, 0, r11, c0, c1, 4\n\t" /* read ID_MMFR0 */
			  /* VMSA */
			  "tst	r11, #0xf\n\t"
			  /* !XN */
			  "movne r6, # 0xc | 0x02\n\t"
			  "blne	__setup_mmu\n\t"
		"__setup_mmu:\n\t"
			  /* r4 pointer to start execute address for kernel */
			  "mov	r4, #0x8000\n\t"
			  /* Page directory size */
			  "sub	r3, r4, #16384\n\t"
			  /* Align the pointer */
			  "bic	r3, r3, #0xff\n\t"
			  "bic	r3, r3, #0x3f00\n\t"
			  /*
			   * Initialise the page tables, turning on the
			   * cacheable and bufferable bits for the RAM area
			   * only.
			   */
			  "mov	r0, r3\n\t"
			  "mov	r9, r0, lsr #18\n\t"
			  /* start of RAM */
			  "mov	r9, r9, lsl #18\n\t"
			  /* a reasonable RAM size */
			  "add	r10, r9, #0x10000000\n\t"
			  /* XN|U + section mapping */
			  "mov	r1, #0x12\n\t"
			  /* AP=11 */
			  "orr	r1, r1, #3 << 10\n\t"
			  /* Setup new page table */
			  "mov	r0, %1\n\t"
			  "add	r2, r0, #16384\n\t"
		"1:\n\t"
			  /* if virt > start of RAM */
			  "cmp	r1, r9\n\t"
			  /* && end of RAM > virt */
			  "cmphs r10, r1\n\t"
			  /* clear XN|U + C + B */
			  "bic	r1, r1, #0x1c\n\t"
			  /* Set XN|U for non-RAM */
			  "orrlo r1, r1, #0x10\n\t"
			  /* Set RAM section settings */
			  "orrhs r1, r1, r6\n\t"
			  /* 1:1 mapping */
			  "str	r1, [r0], #4\n\t"
			  "mov	%0, r1\n\t"
			  "add	r1, r1, #1048576\n\r"
			  "teq	r0, r2\n\t"
			  "bne	1b\n\t"
			: "=r" (D0)
			: "r"  (pgd)
			: "r0", "r1", "r3", "r4", "r6", "r9", "r10", 
			  "r11", "r2"
			);

	printk("D0: %#lx\n", D0);

	/* Dump Page table */
	for (index = 0; index < PAGE_SIZE; index++)
		printk("%04ld: %#08lx\n", index, pgd[index]);

	/* free memory */
	kfree(pgd);

	return 0;
}
device_initcall(debug_zImage_page_table);
