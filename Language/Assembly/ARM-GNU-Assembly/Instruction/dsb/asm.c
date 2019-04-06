/*
 * DSB, Data Synchronization Barrier 
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
 * Data Synchronization Barrier (DSB)
 *
 *   The DSB instruction is a special memory barrier, that synchronizes the 
 *   execution stream with memory accesses. The DSB instruction takes the 
 *   required shareability domain and required access types as arguments, 
 *   see Shareability and access limitations on the data barrier operations.
 *   If the required shareability is Full system then the operation applies 
 *   to all observers within the system.
 *
 *   A DSB behaves as a DMB with the same arguments, and also has the 
 *   additional properties defined here. A DSB completes when:
 *
 *   •    all explicit memory accesses that are observed by Pe before the 
 *        DSB is executed, are of the required access types, and are from
 *        observers in the same required shareability domain as Pe, are 
 *        complete for the set of observers in the required shareability
 *        domain.
 *   •    if the required accesses types of the DSB is reads and writes, all
 *        cache and branch predictor maintenance operations issued by Pe 
 *        before the DSB are complete for the required shareability domain.
 *   •    if the required accesses types of the DSB is reads and writes, all
 *        TLB maintenance operations issued by Pe before the DSB are complete
 *        for the required shareability domain.
 *
 *   In addition, no instruction that appears in program order after the DSB
 *   instruction can execute until the DSB completes.
 * 
 *   ---Note---
 *   Historically, this operation was referred to as Drain Write Buffer or
 *   Data Write Barrier (DWB). From ARMv6, these names and the use of DWB 
 *   were deprecated in favor of the new Data Synchronization Barrier name
 *   and DSB abbreviation. DSB better reflects the functionality provided
 *   from ARMv6, because DSB is architecturally defined to include all cache,
 *   TLB and branch prediction maintenance operations as well as explicit
 *   memory operations.
 *   ----------
 *
 * CP15DSB: Data and instruction barrier operations
 *
 *   •   The ARM and Thumb instruction sets include instruction to perform
 *       the barrier operations, that can be executed at any level of 
 *       privilege.
 *   •   The CP15 c7 operations are defined as write-only operations, that 
 *       can be executed at any level of privilege. The MCR instruction that
 *       performs a barrier operation specifies a register, Rt, as an argument.
 *       However, the operation ignores the value of this register, and 
 *       software does not have to write a value to the register before issuing
 *       the MCR instruction.
 *       In ARMv7, ARM deprecates any use of these CP15 c7 operations, and 
 *       strongly recommends that software uses the ISB,DSB, and DMB
 *       instructions instead.
 *
 *   If the implementation support the SCTLR.CP15BEN bit and this bit is set
 *   to 0, these operations are disabled and their encodings are UNDEFINED. 
 */

int debug_DSB(void)
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

	/* armv6: Data and instruction barrier operations, DSB write-only */
	__asm__ volatile ("mcr p15, 0, r10, c7, c10, 4");
	/* armv7: DSB */
	__asm__ volatile ("dsb");

/*
 * DSB Assembler syntax
 *
 *   DSB{<c>}{<q>} {<option>}
 *
 *   <c>, <q>    See Standard assembler syntax fields. An ARM DSB instruction 
 *               must be unconditional.
 *
 *   <option>    Specifies an optional limitation on the DMB operation. Values
 *               are:
 *
 *               SY       Full system is the required shareability domain, 
 *                        reads and writes are the required access types. 
 *                        Can be omitted. This option is referred to as the 
 *                        full system DMB. Encoded as option = 0b1111.
 *               ST       Full system is the required shareability domain, 
 *                        writes are the required access type. SYST is a 
 *                        synonym for ST . Encoded as option = 0b1110.
 *               ISH      Inner Shareable is the required shareability domain,
 *                        reads and writes are the required access types. 
 *                        Encoded as option = 0b1011.
 *               ISHST    Inner Shareable is the required shareability domain,
 *                        writes are the required access type. Encoded as 
 *                        option = 0b1010.
 *               NSH      Non-shareable is the required shareability domain,
 *                        reads and writes are the required access types. 
 *                        Encoded as option = 0b0111.
 *               NSHST    Non-shareable is the required shareability domain,
 *                        writes are the required access type. Encoded as 
 *                        option = 0b0110.
 *               OSH      Outer Shareable is the required shareability domain,
 *                        reads and writes are the required access types. 
 *                        Encoded as option = 0b0011.
 *               OSHST    Outer Shareable is the required shareability domain,
 *                        writes are the required access type. Encoded as 
 *                        option = 0b0010.
 *
 *   All other encodings of option are reserved. It is IMPLEMENTATION DEFINED
 *   whether options other than SY are implemented. All unsupported and 
 *   reserved options must execute as a full system DSB operation, but 
 *   software must not rely on this behavior.
 */

	/* DSB SY */
	__asm__ volatile ("dsb sy");
	/* DSB ST */
	__asm__ volatile ("dsb st");
	/* DSB ISH */
	__asm__ volatile ("dsb ish");
	/* DSB ISHST */
	__asm__ volatile ("dsb ishst");
	/* DSB NSH */
	__asm__ volatile ("dsb nsh");
	/* DSB NSHST */
	__asm__ volatile ("dsb nshst");
	/* DSB OSH */
	__asm__ volatile ("dsb osh");
	/* DSB OSHST */
	__asm__ volatile ("dsb oshst");

	return 0;
}
device_initcall(debug_DSB);
