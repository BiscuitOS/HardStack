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
 * ID_MMFR0, Memory Model Feature Register 0, VMSA
 *
 *   The ID_MMFR0 characteristics are:
 *
 * **Purpose**
 *
 * - ID_MMFR0 provides information about the implemented memory model and 
 *   memory management support.
 * - This register is a CPUID register, and is part of the Identification 
 *   registers functional group.
 *
 * **Usage constraints**
 *
 * - Only accessible from PL1 or higher.
 * - Must be interpreted with ID_MMFR1, ID_MMFR2, and ID_MMFR3.
 *
 * **Configurations**
 *
 * - The VMSA and PMSA definitions of the register fields are identical.
 * - In a VMSA implementation that includes the Security Extensions, this 
 *   is a Common register.
 *
 *
 * The ID_MMFR0 bit assignments are:
 *                                                       
 *                                                     VMSA Support --o
 *                                        PMSA Support --o            |
 *                    Outermost shareability --o         |            |
 *              Shareability levels --o        |         |            |
 *             TCM Support --o        |        |         |            |
 *                           |        |        |         |            |
 * 31    27      23      19  V    15  V    11  V    7    V      3     V    0
 * +-----+-------+-------+--------+--------+--------+-----------+----------+
 * |     |       |       |        |        |        |           |          |
 * |     |       |       |        |        |        |           |          |
 * |     |       |       |        |        |        |           |          |
 * +-----+-------+-------+--------+--------+--------+-----------+----------+
 *   A       A       A
 *   |       |       |
 *   |       |       o-- Auxiliary register
 *   |       o-- FCSE Supoort
 *   o-- Innermost shareability
 *
 *
 * • Innermost shareability, bits[31:28]
 *
 *   Indicates the innermost shareability domain implemented. Permitted 
 *   values are:
 *
 *   0b0001      Implemented as Non-cacheable.
 *   0b1111      Implemented with hardware coherency support.
 *   0b0000      Shareability ignored.
 *
 *   This field is valid only if the implementation distinguishes between 
 *   Inner Shareable and Outer Shareable, by implementing two levels of 
 *   shareability, as indicated by the value of the Shareability levels 
 *   field, bits[15:12].
 *   When the Shareability levels field is zero, this field is reserved, UNK.
 *
 * • FCSE support, bits[27:24]
 *
 *   Indicates whether the implementation includes the FCSE. Permitted 
 *   values are:
 *
 *   0b0000      Not supported.
 *   0b0001      Support for FCSE.
 *   
 *   The value of 0b0001 is only permitted when the VMSA_support field has
 *   a value greater than 0b0010.
 *
 * • Auxiliary registers, bits[23:20]
 *
 *   Indicates support for Auxiliary registers. Permitted values are:
 *
 *   0b0000      None supported.
 *   0b0001      Support for Auxiliary Control Register only.
 *   0b0010      Support for Auxiliary Fault Status Registers (AIFSR 
 *               and ADFSR) and Auxiliary Control Register.
 *
 * • TCM support, bits[19:16]
 *
 *   Indicates support for TCMs and associated DMAs. Permitted values are:
 *
 *   0b0000    Not supported.
 *   0b0001    Support is IMPLEMENTATION DEFINED . ARMv7 requires this 
 *             setting.
 *   0b0010    Support for TCM only, ARMv6 implementation.
 *   0b0011    Support for TCM and DMA, ARMv6 implementation.
 *
 *   ---Note---
 *   An ARMv7 implementation might include an ARMv6 model for TCM support.
 *   However, in ARMv7 this is an IMPLEMENTATION DEFINED option, and 
 *   therefore it must be represented by the 0b0001 encoding in this field.
 *   ----------
 *
 * • Shareability levels, bits[15:12]
 *
 *   Indicates the number of shareability levels implemented. Permitted
 *   values are:
 *
 *   0b0000      One level of shareability implemented.
 *   0b0001      Two levels of shareability implemented.
 *
 * • Outermost shareability, bits[11:8]
 *
 *   Indicates the outermost shareability domain implemented. Permitted 
 *   values are:
 * 
 *   0b0000      Implemented as Non-cacheable.
 *   0b0001      Implemented with hardware coherency support.
 *   0b1111      Shareability ignored.
 *
 * • PMSA support, bits[7:4]
 *
 *   Indicates support for a PMSA. Permitted values are:
 *
 *   0b0000      Not supported.
 *   0b0001      Support for IMPLEMENTATION DEFINED PMSA.
 *   0b0010      Support for PMSAv6, with a Cache Type Register implemented.
 *   0b0011      Support for PMSAv7, with support for memory subsections. 
 *               ARMv7-R profile.
 *   When the PMSA support field is set to a value other than 0b0000 the 
 *   VMSA support field must be set to 0b0000 .
 *
 * • VMSA support, bits[3:0]
 *
 *   Indicates support for a VMSA. Permitted values are:
 *
 *   0b0000      Not supported.
 *   0b0001      Support for IMPLEMENTATION DEFINED VMSA.
 *   0b0010      Support for VMSAv6, with Cache and TLB Type Registers 
 *               implemented.
 *   0b0011      Support for VMSAv7, with support for remapping and the 
 *               Access flag. ARMv7-A profile.
 *   0b0100      As for 0b0011 , and adds support for the PXN bit in the 
 *               Short-descriptor translation table format descriptors.
 *   0b0101      As for 0b0100 , and adds support for the Long-descriptor
 *               translation table format.
 *   When the VMSA support field is set to a value other than 0b0000 the 
 *   PMSA support field must be set to 0b0000.
 */

static int debug_ID_MMFR0(void)
{
	unsigned long ID_MMFR0;
	unsigned long MMFR0;

	/* Read ID_MMFR0 */
	__asm__ volatile ("mrc p15,0,%0,c0,c1,4" : "=r" (ID_MMFR0));
	printk("ID_MMFR0: %#lx\n", ID_MMFR0);
	MMFR0 = read_cpuid_ext(CPUID_EXT_MMFR0);
	printk("read_cpuid_ext(CPUID_EXT_MMFR0): %#lx\n", MMFR0);

	return 0;
}
device_initcall(debug_ID_MMFR0);
