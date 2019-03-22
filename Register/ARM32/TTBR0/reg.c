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
 * TTBR0, Translation Table Base Register 0, VMSA
 *  
 * The TTBR0 characteristics are:
 *
 * **Purpose**
 *
 * - TTBR0 holds the base address of translation table 0, and information
 *   about the memory it occupies. This is one of the translation tables 
 *   for the stage 1 translation of memory accesses from modes other than
 *   Hyp mode.
 * - This register is part of the Virtual memory control registers 
 *   functional group.
 *
 * **Usage constraints**
 *
 * - Only accessible from PL1 or higher.
 * - Used in conjunction with the TTBCR. When the 64-bit TTBR0 format is 
 *   used, cacheability and shareability information is held in the TTBCR, 
 *   not in TTBR0.
 *
 * **Configurations**
 *
 * - The Multiprocessing Extensions change the TTBR0 32-bit register format.
 * - The Large Physical Address Extension extends TTBR0 to a 64-bit register.
 *   In an implementation that includes the Large Physical Address Extension, 
 *   TTBCR.EAE determines which TTBR0 format is used:
 * 
 *   EAE==0       32-bit format is used. TTBR0[63:32] are ignored.
 *   EAE==1       64-bit format is used.
 *
 * - If the implementation includes the Security Extensions, this register:
 *
 *   • is Banked
 *   • has write access to the Secure copy of the register disabled when the
 *     CP15SDISABLE signal is asserted HIGH.
 *
 * **Attributes**
 *
 * - A 32-bit or 64-bit RW register with a reset value that depends on the 
 *   register implementation. For more information see the register bit 
 *   descriptions. See also Reset behavior of CP14 and CP15 registers.
 *
 *
 * 32-bit TTBR0 format
 *
 *   In an implementation that does not include the Multiprocessing 
 *   Extensions, the 32-bit TTBR0 bit assignments are:
 *
 *   31                                  x               6  5    3  2  1  0
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *   |                                   |   Reserved    |  |    |  |  |  |
 *   | Translation table base 0 address  |   UNK/SBZP    |  |RGN |  |S |C |
 *   |                                   |               |  |    |  |  |  |
 *   +-----------------------------------+---------------+--+----+--+--+--+
 *                                                        A       A
 *                                                        |       |
 *                                                NOS-----o       |
 *                                                IMP-------------o
 *
 *   • Bits[31:x] Translation table base 0 address, bits[31:x]
 *
 *     The value of x determines the required alignment of the translation
 *     table, which must be aligned to (2^x) bytes.
 *
 *   • Bits[x-1:6], ARMv7-A without Multiprocessing Extensions 
 *     
 *     Reserved, UNK/SBZP.     
 *
 *   • NOS, bit[5]
 *
 *     Not Outer Shareable bit. Indicates the Outer Shareable attribute for
 *     the memory associated with a translation table walk that has the 
 *     Shareable attribute, indicated by TTBR0.S == 1:
 *
 *     0        Outer Shareable
 *     1        Inner Shareable.
 *
 *     This bit is ignored when TTBR0.S == 0.
 *     ARMv7 introduces this bit. If an implementation does not distinguish 
 *     between Inner Shareable and Outer Shareable, this bit is UNK/SBZP.
 *
 *   • RGN, bits[4:3] 
 *
 *     Region bits. Indicates the Outer cacheability attributes for the 
 *     memory associated with the translation table walks:
 *
 *     0b00     Normal memory, Outer Non-cacheable.
 *     0b01     Normal memory, Outer Write-Back Write-Allocate Cacheable.
 *     0b10     Normal memory, Outer Write-Through Cacheable.
 *     0b11     Normal memory, Outer Write-Back no Write-Allocate Cacheable.
 *
 *   • IMP, bit[2] 
 *
 *     The effect of this bit is IMPLEMENTATION DEFINED . If the translation
 *     table implementation does not include any IMPLEMENTATION DEFINED 
 *     features this bit is UNK/SBZP.
 *
 *   • S, bit[1]
 *
 *     Shareable bit. Indicates the Shareable attribute for the memory 
 *     associated with the translation table walks:
 *    
 *     0        Non-shareable
 *     1        Shareable.
 *
 *   • C, bit[0], ARMv7-A without Multiprocessing Extensions 
 *
 *     Cacheable bit. Indicates whether the translation table walk is to 
 *     Inner Cacheable memory.
 *
 *     0        Inner Non-cacheable
 *     1        Inner Cacheable.
 *
 *     For regions marked as Inner Cacheable, it is IMPLEMENTATION DEFINED
 *     whether the read has the Write-Through, Write-Back no Write-Allocate,
 *     or Write-Back Write-Allocate attribute.
 */

static int debug_TTBR0(void)
{
	unsigned long TTBR0;

	/* Read TTBR0 */
	__asm__ volatile ("mrc p15,0,%0,c2,c0,0" : "=r" (TTBR0));

	printk("TTBR0: %#lx\n", TTBR0);

	return 0;
}
device_initcall(debug_TTBR0);
