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
 * TTBCR, Translation Table Base Control Register, VMSA
 *
 * **Purpose**
 *
 * - TTBCR determines which of the Translation Table Base Registers, TTBR0 
 *   or TTBR1, defines the base address for a translation table walk 
 *   required for the stage 1 translation of a memory access from any mode
 *   other than Hyp mode.
 * - If the implementation includes the Large Physical Address Extension, 
 *   the TTBCR also:
 *   
 *   • Controls the translation table format.
 *   • When using the Long-descriptor translation table format, holds 
 *     cacheability and shareability information for the accesses.
 *
 *     ---Note---
 *     When using the Short-descriptor translation table format, TTBR0 and
 *     TTBR1 hold this cacheability and shareability information.
 *     ----------
 *
 * - This register is part of the Virtual memory control registers 
 *   functional group.
 *
 * **Usage constraints**
 *
 * - Only accessible from PL1 or higher.
 *
 * **Configurations**
 *
 * - The Large Physical Address Extension adds an alternative format for 
 *   the register. If an implementation includes the Large Physical Address
 *   Extension then the current translation table format determines which 
 *   format of the register is used.
 * - If the implementation includes the Security Extensions, this register:
 *
 *   • is Banked
 *   • has write access to the Secure copy of the register disabled when
 *     the CP15SDISABLE signal is asserted HIGH.
 *
 * **Attributes**
 *
 * - A 32-bit RW register that resets to zero. If the implementation 
 *   includes the Security Extensions this defined reset value applies only
 *   to the Secure copy of the register, except for the EAE bit in an 
 *   implementation that includes the Large Physical Address Extension. For
 *   more information see the field descriptions. See also Reset behavior 
 *   of CP14 and CP15 registers on page B3-1450.
 *
 * TTBCR format when using the Short-descriptor translation table format
 *
 *   In an implementation that includes the Security Extensions and is using
 *   the Short-descriptor translation table format, the TTBCR bit assignments
 *   are:
 *   31 30                                               5  4  3  2    0
 *   +--+------------------------------------------------+--+--+--+----+
 *   |  |                                                |  |  |  |    |
 *   |  |       Reserved. UNK/SBZP                       |  |  | 0| N  |
 *   |  |                                                |  |  |  |    |
 *   +--+------------------------------------------------+--+--+--+----+
 *    EAE                                                 A  A
 *                                                        |  |
 *                                               PD1------o  |
 *                                               PD0---------o
 *
 *   In an implementation that does not include the Security Extensions, 
 *   and is using the Short-descriptor translation table format, the TTBCR 
 *   bit assignments are:
 *
 *   31 30                                                        2    0
 *   +--+---------------------------------------------------------+----+
 *   |  |                                                         |    |
 *   |  |       Reserved. UNK/SBZP                                | N  |
 *   |  |                                                         |    |
 *   +--+---------------------------------------------------------+----+
 *
 *   • EAE, bit[31], if implementation includes the Large Physical Address 
 *     Extension.
 *
 *     Extended Address Enable. The meanings of the possible values of this
 *     bit are:
 *
 *     0       Use the 32-bit translation system, with the Short-descriptor
 *             translation table format. In this case, the format of the 
 *             TTBCR is as described in this section.
 *     1       Use the 40-bit translation system, with the Long-descriptor 
 *             translation table format. In this case, the format of the 
 *             TTBCR is as described in TTBCR format when using the 
 *             Long-descriptor translation table format.
 *    
 *     This bit resets to 0, in both the Secure and the Non-secure copies
 *     of the TTBCR.
 *
 *   • Bits[30:6, 3] Reserved, UNK/SBZP.
 *
 *   • PD1, bit[5], in an implementation that includes the Security 
 *     Extensions
 *
 *     Translation table walk disable for translations using TTBR1. This 
 *     bit controls whether a translation table walk is performed on a TLB
 *     miss, for an address that is translated using TTBR1. The encoding
 *     of this bit is:
 * 
 *     0       Perform translation table walks using TTBR1.
 *     1       A TLB miss on an address that is translated using TTBR1 
 *             generates a Translation fault. No translation table walk 
 *             is performed.
 *
 *   • PD0, bit[4], in an implementation that includes the Security 
 *     Extensions
 *
 *     Translation table walk disable for translations using TTBR0. This 
 *     bit controls whether a translation table walk is performed on a TLB
 *     miss for an address that is translated using TTBR0. The meanings
 *     of the possible values of this bit are equivalent to those for the
 *     PD1 bit.
 *
 *   • Bits[5:4], in an implementation that does not include the 
 *     Security Extensions
 *
 *     Reserved, UNK/SBZP.
 *
 *   • N, bits[2:0]
 *
 *     Indicate the width of the base address held in TTBR0. In TTBR0, 
 *     the base address field is bits[31:14-N]. The value of N also 
 *     determines:
 *
 *     - whether TTBR0 or TTBR1 is used as the base address for 
 *       translation table walks.
 *     - the size of the translation table pointed to by TTBR0.
 *   
 *     N can take any value from 0 to 7, that is, from 0b000 to 0b111. 
 */

static int debug_TTBCR(void)
{
	unsigned long TTBCR;
	unsigned long TTBR0;
	unsigned long TTBR1;

	/* Read TTBCR */
	__asm__ volatile ("mrc p15,0,%0,c2,c0,2" : "=r" (TTBCR));
	__asm__ volatile ("mrc p15,0,%0,c2,c0,0" : "=r" (TTBR0));
	__asm__ volatile ("mrc p15,0,%0,c2,c0,1" : "=r" (TTBR1));

	printk("TTBCR: %#lx\n", TTBCR);
	printk("TTBR0: %#lx\n", TTBR0);
	printk("TTBR1: %#lx\n", TTBR1);

	return 0;
}
device_initcall(debug_TTBCR);
