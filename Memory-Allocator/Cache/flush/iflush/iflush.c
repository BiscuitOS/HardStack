/*
 * iflush
 *
 * (C) 2019.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * Cache assignementation:
 *
 *
 *                    Cache Way/Set                   
 *                   | <-------> |
 *                -- +-+-+-+-+-+-+-+-+-+-+-+-+ --
 *                A  | | | | | | | | | | | | |  A
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *      Cache Tag |  | | | | | | | | | | | | |  |
 *           flag |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                V  | | | | | | | | | | | | |  |
 *                -- +-+-+-+-+-+-+-+-+-+-+-+-+  | Cache Line Frame
 *                A  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *     Cache Line |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                |  | | | | | | | | | | | | |  |
 *                V  | | | | | | | | | | | | |  V
 *                -- +-+-+-+-+-+-+-+-+-+-+-+-+ --
 *                   0                       32K
 *
 *  • Cache Line
 *    
 *    Mini matedata on Cache that contain data from Main-Memory.
 *
 *  • Cache Set
 *
 *    Each level of a cache is split up into a number of sets.
 *
 *  • Cache Way
 *
 *    The Associativity of a cache defines the number of location in a set
 *    to which an address can be assigned.
 */

static int debug_iflush(void)
{

	/* CP15DSB: Data Synchronization Barrier operation */
	__asm__ volatile ("mcr p15, 0, r10, c7, c10, 4");

	/*
         * ICIALLU:
	 *   Invalidate all instruction caches to PoU. If branch predictors
	 *   are architecturally-visible, also flushes branch predictors. 
	 */
	__asm__ volatile ("mcr p15, 0, r10, c7, c5, 0");

	/* DSB: Data Synchronization Barrier operation 
	 *
	 *   DSB is a memory barrier that ensure the completion of
	 *   memory accesses.
	 */
	__asm__ volatile ("mcr p15, 0, r10, c7, c10, 4");

	/* ISB: Instruction Synchronization Barrier operation.
	 *
	 *   ISB flushes the pipeline in the processor, so the all
	 *   instructions following the ISB are fetched from cache
	 *   or memory, after the instruction has been completed.
	 */
	__asm__ volatile ("mcr p15, 0, r10, c7, c5, 4");

	printk("iflush.....\n");

	return 0;
}
device_initcall(debug_iflush);
