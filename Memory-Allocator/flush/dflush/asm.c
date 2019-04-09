/*
 * Flush Data cache
 *
 * (C) 2019.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

static int debug_dflush(void)
{
	unsigned long D0;

	__asm__ volatile ("mcr	p15, 0, r10, c7, c10, 5\n\t" /* DMB */
			  /* Read clidr */
			  "mrc	p15, 1, r0, c0, c0, 1\n\t"
			  /* extract Level of Coherence(LoC) for the cache
			   * from clidr.
			   */
			  "ands	r3, r0, #0x7000000\n\t"
			  /* left align LoC bit field */
			  "mov	r3, r3, lsr #23\n\t"
			  /* if LoC is 0, then no need to clean */
			  "beq finished\n\t"
			  /* start clean at cache level 0 (index = 0) */
			  "mov	r10, #0\n\t"
		"loop1:\n\t"
			  /* Work out (3 * index) current level */
			  "add	r2, r10, r10, lsr #1\n\t"
			  /* extract cache type bits from clidr */
			  "mov	r1, r0, lsr r2\n\t"
			  /* mask of the bits for current cache only */
			  "and	r1, r1, #7\n\t"
			  /* see what cache we have at this level */
			  "cmp	r1, #2\n\t"
			  /* skip if no cache, or just i-cache */
			  "blt	skip\n\t"
			  /* select current cache level in cssr */
			  "mcr	p15, 2, r10, c0, c0, 0\n\t"
			  /* ISB to sychronization the new cssr&csidr */
			  "mcr	p15, 0, r10, c7, c5, 4\n\t"
			  /* read the new csidr */
			  "mrc	p15, 1, r1, c0, c0, 0\n\t"
			  /* extract the length of the cacehe lines */
			  "and	r2, r1, #7\n\t"
			  /* add 4 (line length offset) */
			  "add	r2, r2, #4\n\t"
			  /* Set constant value 0x3ff to r4 */
			  "ldr	r4, =0x3ff\n\t"
			  /* find maximum number on the way size */
			  "ands	r4, r4, r1, lsr #3\n\t"
			  /* find bit position of way size increment */
			  "clz	r5, r4\n\t"
			  /* Set constant value 0x7fff to r7 */
			  "ldr	r7, =0x7fff\n\t"
			  /* extract max number of the index size */
			  "ands	r7, r7, r1, lsr #13\n\t"
		"loop2:\n\t"
			  /* create working copy of max way size */
			  "mov	r9, r4\n\t"
		"loop3:\n\t"
			  /* factor way and cache number */
			  "orr	r11, r10, r9, lsl r5\n\t"
			  /* factor index number into r11 */
			  "orr	%0, r11, r7, lsl r2\n\t"
			  /* clean & invalidate by set/way */
			  "mcr	p15, 0, r11, c7, c14, 2\n\t"
			  /* decrement the way */
			  "subs	r9, r9, #1\n\t"
			  "bge	loop3\n\t"
			  /* decrement the index */
			  "subs r7, r7, #1\n\t"
			  "bge	loop2\n\t"
		"skip:\n\t"
			  /* increment cache number */
			  "add	r10, r10, #2\n\t"
			  "cmp	r3, r10\n\t"
			  "bgt	loop1\n\t"
		"finished:\n\t"
			  /* switch back to cache level 0 */
			  "mov	r10, #0\n\r"
			  /* select current cache level in cssr */
			  "mcr	p15, 2, r10, c0, c0, 0\n\t"
			  : "=r" (D0)
			  :
			  : "r0", "r1", "r2", "r3", "r4", "r5", "r7", 
			    "r9", "r10", "r11");

	printk("D0: %#lx\n", D0);

	return 0;
}
device_initcall(debug_dflush);
