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
 * CSSELR, Cache Size Selection Register, VMSA
 *
 * **Purpose**
 *
 *   The CSSELR selects the current CCSIDR, by specifying:
 *  
 *   •    The required cache level.
 *   •    The cache type, either:
 *
 *        ----   Instruction cache, if the memory system implements 
 *               separate instruction and data caches.
 *        ----   Data cache. The data cache argument must be used for
 *               a unified cache.
 *
 *   This register is part of the Identification registers functional group.
 *
 * **Usage constraints**
 *
 *   Only accessible from PL1 or higher.
 *
 * **Configurations**
 *
 *   This register is not implemented in architecture versions before ARMv7.
 *   If the implementation includes the Security Extensions, this register
 *   is Banked.
 *
 * **Attributes**
 *
 *   A 32-bit RW register with an UNKNOWN reset value. See also Reset behavior
 *   of CP14 and CP15 registers.
 *
 * The CSSELR bit assignments are:
 *
 * 32                                                       4       1 0
 * +--------------------------------------------------------+-------+-+
 * |                                                        |       | |
 * |                 Reserved, UNK/SBZP                     | level | |
 * |                                                        |       | |
 * +--------------------------------------------------------+-------+-+
 *                                                                   A
 *                                                                   |
 *                                                           InD ----o
 *
 *
 *
 * Bits[31:4]
 *
 *   Reserved, UNK/SBZP.
 *
 * Level, bits[3:1]
 *
 *   Cache level of required cache. Permitted values are from 0b000, 
 *   indicating Level 1 cache, to 0b110 indicating Level 7 cache. If
 *   this field is set to an unimplemented level of cache, the effect
 *   is UNPREDICTABLE.
 *
 * InD, bit[0]
 *
 *   Instruction not Data bit. Permitted values are:
 *
 *   0     Data or unified cache
 *   1     Instruction cache.
 *
 * See the Note in Access to registers from Monitor mode on page B3-1459 for
 * a description of how SCR.NS controls whether Monitor mode accesses are to
 * the Secure or Non-secure copy of the selected CCSIDR.
 */

static int debug_CSSELR(void)
{
	unsigned long CSSELR;

	/* Read CSSELR registor. */
	__asm__ volatile ("mrc p15, 2, %0, c0, c0, 0" : "=r" (CSSELR));

	printk("Cache level of required cache: ctype%ld\n", 
		          ((CSSELR >> 1) & 0x7) + 1);

	/* Detect whether cache is instruction cache or Data cache. */
	if (CSSELR & 0x1) {
		printk("Instruction cache.\n");
	} else {
		printk("Data or unified cache.\n");
	}

	return 0;
}
device_initcall(debug_CSSELR);
