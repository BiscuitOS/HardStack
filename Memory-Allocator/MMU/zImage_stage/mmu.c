/*
 * ARMv7 MMU on zImage stage
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

/*
 * MMU Scheme (zImage stage)
 *
 *
 * Page Table (1:1 16K - 4G Virtual Space)
 *
 * +----------+----+---+-+-+-+-+-+ ---
 * |          | AP |   |0|B|C|1|0|   A
 * +----------+----+---+-+-+-+-+-+   |
 * |          | AP |   |0|B|C|1|0|   |
 * +----------+----+---+-+-+-+-+-+   | 256M RAM Section Mapping
 * |          .....              |   |
 * +----------+----+---+-+-+-+-+-+   |
 * |          | AP |   |0|B|C|1|0|   V
 * +----------+----+---+-+-+-+-+-+ ---
 * |          | AP |   |1|B|C|1|0|   A
 * +----------+----+---+-+-+-+-+-+   |
 * |          | AP |   |1|B|C|1|0|   |
 * +----------+----+---+-+-+-+-+-+   | (4G - 256) Non-RAM Section Mapping
 * |          .....              |   |
 * +----------+----+---+-+-+-+-+-+   |
 * |          | AP |   |1|B|C|1|0|   V
 * +----------+----+---+-+-+-+-+-+ ---
 *              |
 *              |
 *              |
 *              |                     SCTLR, System Contrl Register
 *              |                     32                                  0
 *              |                     +---------------+-+--+--+-+-+-+-+-+-+
 *              |                     |               |U|  |RR| |I| |C|A|M|   
 *              |                     +---------------+-+--+--+-+-+-+-+-+-+
 *              |                              |
 *              |                              |
 *              |                              | Enable MMU/Icache/Dcache
 *              |                              | Alignement check
 *              |                              | 
 *              |                              |
 *              | Base Address of              |
 *              | Page Table (PA)              |
 * TTBR0        |                              |
 * 32           V                  0           |
 * +-------------------------------+           |
 * |                               |           |
 * +-------------------------------+           |
 *              |                              |
 *              |                              |
 * TTBCR        |                              |
 * 32           V                              |
 * +-+--------------------+---+----+           |
 * |0|                    |PD0|    |           |
 * +-+--------------------+---+----+           |
 *              |                              |
 *              | 32-bit translation           |
 *              | Only use TTBR0               |
 *              |                              |
 *              o------------------------o     |
 *                                       |     |
 * DACR                                  |     |
 * 32                              0     |     |
 * +-------------------------+--+--+     |     |
 * |                         |D1|D0|     |     |
 * +-------------------------+--+--+     |     |
 *                                |      |     |
 *                                |      |     |
 *             Domain0 --> client |      |     |
 *                                |      |     |
 *                                o-----(+)    |
 *                                       |     |
 *                                       |     |
 *                                       o-(+)-o
 *                                          |
 *                                          | ISB
 *                                          |
 *                                          |
 *                                          |
 *                                          V 
 *                                    MMU/Cache work
 *                                      
 */ 

static int debug_zImage_MMU(void)
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

	__asm__ volatile (
#ifdef CONFIG_MMU
			  /* read ID_MMFR0 */
			  "mrc	p15, 0, r11, c0, c1, 4\n\t"
			  /* VMSA */
			  "tst	r11, #0xf\n\t"
			  /* !XN */
			  "movne r6, # 0xc | 0x02\n\t"
			  "blne	__setup_mmu\n\t"
			  "b	__setup_done\n\t"
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
			  "add	r1, r1, #1048576\n\r"
			  "teq	r0, r2\n\t"
			  "bne	1b\n\t"
			  /*
			   * If wver we are running from Flash, then we surely
			   * want the cache to be enabled also for our 
			   * execution instance... We map 2MB of it so there
			   * is no map overlap problem for up to 1MB compressed
			   * kernel. If the execution is in RAM then we would
			   * only be duplicating the above.
			   */
			  /* ensure B is set for this */
			  "orr	r1, r6, #0x04\n\t"
			  "orr	r1, r1, #3 << 10\n\t"
			  "mov	r2, pc\n\t"
			  "mov	r2, r2, lsr #20\n\t"
			  "orr	r1, r1, r2, lsl #20\n\t"
			  "add	r0, %1, r2, lsl #2\n\t"
			  "str	r1, [r0], #4\n\t"
			  "add	r1, r1, #1048576\n\t"
			  "str	r1, [r0]\n\t"
		"__setup_done:\n\t"
			  "mov	r0, #0\n\t"
			  /* drain write buffer */
			  "mcr	p15, 0, r0, c7, c10, 4\n\t"
			  /* VMSA */
			  "tst	r11, #0xf\n\t"
			  /* Flush I,D TLBs */
			  "mcrne p15, 0, r0, c8, c7, 0\n\t"
#endif
			  /* read control reg */
			  "mrc	p15, 0, r0, c1, c0, 0\n\t"
			  /* clear SCTLR.TRE */
			  "bic	r0, r0, #1 << 28\n\t"
			  /* I-cache enable, RR cache replacement */
			  "orr	r0, r0, #0x5000\n\t"
			  /* write buffer */
			  "orr	r0, r0, #0x003c\n\t"
			  /* A (no unaligned access fault) */
			  "bic	r0, r0, #2\n\t"
			  /* U (v6 unaligned access model) */
			  /*   (needed for ARM1176) */
			  "orr	r0, r0, #1 << 22\n\t"
#ifdef CONFIG_MMU
			  /* read ttb control reg */
			  "mrcne p15, 0, r6, c2, c0, 2\n\t"
			  /* MMU enabled */
			  "orrne r0, r0, #1\n\t"
			  /* domain 0 = client */
			  "movne r1, #0xfffffffd\n\t"
			  /* 32-bit translation system */
			  "bic	r6, r6, #1 << 31\n\t"
			  /* use only ttbr0 */
			  "bic	r6, r6, #(7 << 0) | (1 << 4)\n\t"
			  /* load page table pointer */
			  "mcrne p15, 0, %1, c2, c0, 1\n\t"
			  /* load domain access control */
			  "mcrne p15, 0, r1, c3, c0, 0\n\t"
			  /* load ttb control */
			  "mcrne p15, 0, r6, c2, c0, 2\n\t"
#endif
			  /* ISB */
			  "mcr	p15, 0, r0, c7, c5, 4\n\t"
			  /* load control register */
			  "mcr	p15, 0, r0, c1, c0, 0\n\t"
			  /* and read it back */
			  "mrc	p15, 0, r0, c1, c0, 0\n\t"
			  "mov	r0, #0\n\t"	
			  /* ISB */		  
			  "mcr	p15, 0, r0, c7, c5, 4\n\t"
			: "=r" (D0)
			: "r"  (pgd)
			: "r0", "r1", "r3", "r4", "r6", "r9", "r10", 
			  "r11", "r2", "pc"
			);

	printk("D0: %#lx\n", D0);

	/* Dump Page table */
	for (index = 0; index < PAGE_SIZE; index++)
		printk("%04ld: %#08lx\n", index, pgd[index]);

	/* free memory */
	kfree(pgd);

	return 0;
}
device_initcall(debug_zImage_MMU);
