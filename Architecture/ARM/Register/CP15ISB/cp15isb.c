/*
 * CP15ISB, Instruction Synchronization Barrier
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
 * Instruction Synchronization Barrier (ISB)
 *
 *   An ISB instruction flushes the pipeline in the processor, so that all
 *   instructions that come after the ISB instruction in program order are
 *   fetched from cache or memory only after the ISB instruction has 
 *   completed. Using an ISB ensures that the effects of context-changing 
 *   operations executed before the ISB are visible to the instructions
 *   fetched after the ISB instruction. Examples of context-changing 
 *   operations that require the insertion of an ISB instruction to ensure 
 *   the effects of the operation are visible to instructions fetched after
 *   the ISB instruction are:
 *
 *   •    completed cache, TLB, and branch predictor maintenance operations
 *   •    changes to system control registers.
 *
 *   Any context-changing operations appearing in program order after the 
 *   ISB instruction only take effect after the ISB has been executed.
 *   For more information about the ISB instruction in the Thumb and ARM 
 *   instruction sets.
 *
 * CP15ISB, Instruction Synchronization Barrier operation
 *
 *   In ARMv7, the ISB instruction performs an Instruction Synchronization
 *   Barrier operation. The deprecated CP15 c7 encoding for an Instruction
 *   Synchronization Barrier is an MCR instruction with <opc1> set to 0,
 *   <CRn> set to c7, <CRM> set to c5, and <cpc2> set to 4.
 */

int debug_ISB(void)
{
	unsigned long SCTLR;

	/* Detect SCTLR.CP15BEN */
	__asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (SCTLR));
	if ((SCTLR >> 5) & 0x1) {
		printk("CP15 barrier operation enable!\n");
	} else {
		printk("CP15 barrier operation disabled. Their encoding are"
			" UNDEFINED.\n");
	}

	/* Instruction Synchronization Barrier operation, ISB write-only */
	__asm__ volatile ("mcr p15, 0, r10, c7, c5, 4");

	return 0;
}
device_initcall(debug_ISB);
