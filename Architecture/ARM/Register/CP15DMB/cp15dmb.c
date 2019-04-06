/*
 * CP15DMB, Data Memory Barrier
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
 * Data Memory Barrier (DMB)
 *
 *   The DMB instruction is a data memory barrier. The processor that
 *   executes the DMB instruction is referred to as the executing 
 *   processor, Pe. The DMB instruction take the required shareability
 *   domain and required access types as arguments. If the required 
 *   shareability is Full system then the operation applies to all
 *   observers within the systtem.
 *
 *   A DMB creates two groups of memory accesses, Group A and Group B:
 *
 *   Group A    contains:
 *              •    All explicit memory accesses of the required access
 *                   types from observers in the same required shareability
 *                   domain as Pe that are observed by Pe before the DMB
 *                   instruction. These accesses include any accesses of
 *                   the required access types performed by Pe.
 *              •    All loads of required access types from an observer
 *                   Px in the same required shareability domain as Pe
 *                   that have been observed by any given different
 *                   observer, Py, in the same required shareability domain
 *                   as Pe before Py has performed a memory access that
 *                   is a member of Group A.
 *
 *   Group B    contains:
 *              •    All explicit memory accesses of the required access
 *                   types by Pe that occur in program order after the DMB
 *                   instruction.
 *              •    All explicit memory accesses of the required access
 *                   types by any given observer Px in the same required
 *                   shareability domain as Pe that can only occur after
 *                   a load by Px has returned the result of a store that
 *                   is a member of Group B.
 *
 *   Any observer with the same required shareability domain as Pe observes
 *   all members of Group A before it observes any memory of Group B to the 
 *   extent that those group members are required to be observed, as 
 *   determined by the shareability and cacheability of the memory locations
 *   accessed by the group members.
 *
 *   Where members of Group A and members of Group B access the same 
 *   memory-mapped peripheral of arbitrary system-defined size, then members 
 *   of Group A that are accessing Strongly-ordered, Device, or Normal
 *   Non-cacheable memory arrive at that peripheral before members of Group 
 *   B that are accessing Strongly-ordered, Device, or Normal Non-cacheable 
 *   memory. If the memory accesses are not to a peripheral, then there are
 *   no restrictions from this paragraph.
 *
 *   ---Note---
 *   •    Where the members of Group A and Group B that must be ordered are 
 *        from the same processor, a DMB NSH is sufficient for this guarantee.
 *   •    A memory access might be in neither Group A nor Group B. The DMB 
 *        does not affect the order of observation of such a memory access.
 *   •    The second part of the definition of Group A is recursive. 
 *        Ultimately, membership of Group A derives from the observation by 
 *        Py of a load before Py performs an access that is a member of Group
 *        A as a result of the first part of the definition of Group A.
 *   •    The second part of the definition of Group B is recursive. 
 *        Ultimately, membership of Group B derives from the observation by
 *        any observer of an access by Pe that is a member of Group B as a 
 *        result of the first part of the definition of Group B.
 *   ----------
 *  
 */

int debug_DMB(void)
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

	/* armv6 Data Memory Barrier operation, DMB write-only */
	__asm__ volatile ("mcr p15, 0, r10, c7, c10, 5");
	/* armv7 Data Memory Barrier operation */
	__asm__ volatile ("dmb");

/*
 * DMB Assembler syntax
 *
 *   DMB{<c>}{<q>} {<option>}
 *
 *   <c>, <q>    See Standard assembler syntax fields. An ARM DMB instruction 
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
 *   reserved options must execute as a full system DMB operation, but 
 *   software must not rely on this behavior.
 */

	/* DMB SY */
	__asm__ volatile ("dmb sy");
	/* DMB ST */
	__asm__ volatile ("dmb st");
	/* DMB ISH */
	__asm__ volatile ("dmb ish");
	/* DMB ISHST */
	__asm__ volatile ("dmb ishst");
	/* DMB NSH */
	__asm__ volatile ("dmb nsh");
	/* DMB OSH */
	__asm__ volatile ("dmb osh");
	/* DMB OSHST */
	__asm__ volatile ("dmb oshst");

	return 0;
}
device_initcall(debug_DMB);
