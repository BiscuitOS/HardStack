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

#include <asm/cputype.h>
/*
 * MIDR, Main ID Register, VMSA
 *
 *   The MIDR characteristics are:
 *
 * **Purpose**
 *
 * - The MIDR provides identification information for the processor, 
 *   including an implementer code for the device and a device ID number.
 * - This register is part of the Identification registers functional 
 *   group.
 *
 * **Usage constraints**
 *
 * - Only accessible from PL1 or higher.
 *
 * **Configurations**
 * 
 * - If the implementation includes the Security Extensions, this register 
 *   is Common. 
 * - Some fields of the MIDR are IMPLEMENTATION DEFINED . For details of the
 *   values of these fields for a particular ARMv7 implementation, and any 
 *   implementation-specific significance of these values, see the product
 *   documentation.
 *
 * **Attributes**
 *
 * - A 32-bit RO register with an IMPLEMENTATION DEFINED value. See also 
 *   Reset behavior of CP14 and CP15 registers on page B3-1450.
 *
 * 
 * The MIDR bit assignments are:
 *
 * 31          23        19             15                    3          0
 * +-----------+---------+--------------+---------------------+----------+
 * |           |         |              |                     |          |
 * | Implement | Variant | Architecture | Primary part number | Revision |
 * |           |         |              |                     |          |
 * +-----------+---------+--------------+---------------------+----------+
 *
 * • Implementer, bits[31:24]
 *
 *   Bits[31:24]    |    ASCII    |    character Implementer
 *   ---------------+-------------+-------------------------
 *   0x41           | A           | ARM Limited
 *   0x44           | D           | Digital Equipment Corporation
 *   0x4D           | M           | Motorola, Freescale Semiconductor Inc.
 *   0x51           | Q           | Qualcomm Inc.
 *   0x56           | V           | Marvell Semiconductor Inc.
 *   0x69           | i           | Intel Corporation
 *   ---------------+-------------+-------------------------
 *
 * • Variant, bits[23:20]
 *
 *   An IMPLEMENTATION DEFINED variant number. Typically, this field 
 *   distinguishes between different product variants, or major revisions 
 *   of a product.
 * 
 * • Architecture, bits[19:16]
 *
 *   Table shows the permitted values for this field:
 *
 *   Bits[19:16]    |    Architecture
 *   ---------------+-------------------------------
 *   0x1            | ARMv4
 *   0x2            | ARMv4T
 *   0x3            | ARMv5 (obsolete)
 *   0x4            | ARMv5T
 *   0x5            | ARMv5TE
 *   0x6            | ARMv5TEJ
 *   0x7            | ARMv6
 *   0xF            | Defined by CPUID scheme
 *   ---------------+-------------------------------
 *
 * • Primary part number, bits[15:4]
 *
 *   An IMPLEMENTATION DEFINED primary part number for the device.
 *
 *   ---Note---
 *   > On processors implemented by ARM, if the top four bits of the 
 *     primary part number are 0x0 or 0x7 , the variant and architecture 
 *     are encoded differently, see the description of the MIDR in 
 *     Appendix D15 ARMv4 and ARMv5 Differences.
 *   > Processors implemented by ARM have an Implementer code of 0x41.
 *   ----------
 *
 * • Revision, bits[3:0]
 *
 *   An IMPLEMENTATION DEFINED revision number for the device.
 *
 * ARMv7 requires all implementations to use the CPUID scheme, described 
 * in Chapter B7 The CPUID Identification Scheme, and an implementation is 
 * described by the MIDR with the CPUID registers.
 *
 * ---Note---
 * For an ARMv7 implementation by ARM, the MIDR is interpreted as:
 * Bits[31:24]            Implementer code, must be 0x41 .
 * Bits[23:20]            Major revision number, rX.
 * Bits[19:16]            Architecture code, must be 0xF .
 * Bits[15:4]             ARM part number.
 * Bits[3:0]              Minor revision number, pY.
 * ----------
 */

static int debug_MIDR(void)
{
	unsigned long CPUID;

	/* Read MIDR */
	__asm__ volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (CPUID));

	printk("MIDR:              %#lx\n", CPUID);
	printk("Implement code:    %#lx\n", CPUID >> 24);
	printk("Revision number:   %#lx - %#lx\n", (CPUID >> 20) & 0xF, 
						 CPUID & 0xF);
	printk("Architecture code: %#lx\n", (CPUID >> 16) & 0xF);
	printk("ARM part number:   %#lx\n", (CPUID >> 4) & 0xFFFF);

	printk("read_cpuid_id():   %#x\n", read_cpuid_id());
	return 0;
}
device_initcall(debug_MIDR);
