/*
 * Arm Register
 *
 * (C) 2019.03.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>

/*
 * CCSIDR, Cache Size ID Registers, VMSA
 *
 * **Purpose**
 *
 *   The CCSIDR provides information about the architecture of the caches.
 *   This register is part of the Identification registers functional group.
 *
 * **Usage constraints**
 *
 *   Only accessible from PL1 or higher.
 *   If CSSELR indicates a cache that is not implemented, the result of 
 *   reading CCSIDR is UNPREDICTABLE .
 *
 * **Configurations**
 *
 *   The implementation includes one CCSIDR for each cache that it can 
 *   access. CSSELR selects which Cache Size ID Register is accessible.
 *   Architecture versions before ARMv7 do not define these registers.
 *   If the implementation includes the Security Extensions, these registers
 *   are Common.
 *
 * **Attributes**
 *
 *   A 32-bit RO register with an IMPLEMENTATION DEFINED value. See also
 *   Reset behavior of CP14 and CP15 registers on page B3-1450.
 *
 *
 * 32                                                               0
 * +-+-+-+-+---------------------+-----------------------+----------+
 * | | | | |                     |                       |          |
 * | | | | |       NumSets       |     Associativity     | LineSize |
 * | | | | |                     |                       |          |
 * +-+-+-+-+---------------------+-----------------------+----------+
 *  | | | |
 *  | | | o--- WA
 *  | | o----- RA
 *  | o------- WB
 *  o--------- WT
 *
 *  WT,bit[31]   Indicates whether the cache level supports write-through.
 *  WB,bit[30]   Indicates whether the cache level supports write-back.
 *  RA,bit[29]   Indicates whether the cache level supports read-allocation
 *  WA,bit[28]   Indicates whether the cache level supports write-allocation
 *
 *  WT,WB,RA, and WA bit value: 0 not support; 1 support
 *
 *  NumSets,bits[27,13]
 *
 *    (Number of sets in cache) - 1, therefore a value of 0 indicates 1 set
 *    in the cache. The number of sets does not have to be a power of 2.
 *
 *  Associativity,bits[12:3]
 *
 *    (Associativity of cache) - 1, therefore a value of 0 indicates an
 *    associativity of 1. The associativity does not have to be a power of 2.
 *
 *  LineSize,bits[2:0]
 *
 *    (Log2(Number of words in cache line)) -2. For example:
 *
 *    •    For a line length of 4 words: Log2(4) = 2, LineSize entry = 0.
 *         This is the minimum line length.
 *    
 *    •    For a line length of 8 words: Log2(8) = 3, lineSize entry = 1.
 *  
 *  ---Note---
 *  The parameters NumSets, Associativity and LineSize in these registers
 *  define the architecturally visible parameters that are required for 
 *  the cache maintenance by Set/Way instructions. They are not guaranteed
 *  to represent the actual microarchitectural features of a design. You
 *  cannot make any inference about the actual sizes fo caches based on
 *  these parameters.
 *  ----------
 */

static int debug_CCSIDR(void)
{
	unsigned long CCSIDR;

	__asm__ volatile ("mrc p15, 1, %0, c0, c0, 0" : "=r" (CCSIDR));

	/* Detect whether support write-through */
	if ((CCSIDR >> 31) & 0x1) {
		printk("Support write-through.\n");
	}

	/* Detect whether support write-back */
	if ((CCSIDR >> 30) & 0x1) {
		printk("Support write-back.\n");
	}

	/* Detect whether support read-allocation */
	if ((CCSIDR >> 29) & 0x1) {
		printk("Support read-allocation.\n");
	}

	/* Detect whether support write-allocation */
	if ((CCSIDR >> 28) & 0x1) {
		printk("Support write-allocation.\n");
	}

	/* Detect NumSets property */
	printk("Cache NumSets:       %#lx\n", (CCSIDR >> 13) & 0x7FFF);

	/* Detect Associativity property */
	printk("Cache Associativity: %#lx\n", (CCSIDR >> 3) & 0x3FF);

	/* Detect LineSize property */
	printk("Cache LineSize:      %#lx\n", CCSIDR & 0x7);

	return 0;
}
device_initcall(debug_CCSIDR);
