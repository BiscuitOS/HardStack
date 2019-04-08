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
 * CTR, Cache Type Register, VMSA
 *
 * **Purpose**
 *
 *   The CTR provides information about the architecture of the caches.
 *   This register is part of the Identification registers functional group.
 *
 * **Usage constraints**
 *
 *   Only accessible from PL1 or higher.
 *
 * **Configurations**
 *
 *   If the implementation includes the Security Extensions, this register
 *   is Common. ARMv7 changes the format of the CTR, This section describes
 *   only the ARMv7 format. For more information see the description of the
 *   Format field, bits[31:29].
 *
 * **Attributes**
 *
 *   A 32-bit RO register with an IMPLEMENTATION DEFINED value. See also
 *   Reset behavior of CP14 and CP15 registers.
 *
 *
 * In an ARMv7, the CTR bit assignments are:
 *
 * 32  29      24    20         16     14                    4          0
 * +---+-+-----+-----+----------+------+---------------------+----------+
 * |   | |     |     |          |      |                     |          |
 * |100|0| CWG | ERG | DminLine | L1lp | 0 0 0 0 0 0 0 0 0 0 | IminLine |
 * |   | |     |     |          |      |                     |          |
 * +---+-+-----+-----+----------+------+---------------------+----------+
 *
 *
 *
 * Format, bits[31:29]
 *
 *   Indicates the implemented CTR format. The possible values of this are:
 *
 *   0b000     ARMv6 format, see CP15 c0, Cache Type Register, CTR, ARMv4 
 *             and ARMv5.
 *   0b100     ARMv7 format. This is the format described in this section.
 *
 * Bit[28]
 *
 *   RAZ.
 *
 * CWG, bits[27:24]
 *
 *   Cache Write-back Granule. The maximum size of memory that can be
 *   overwritten as a result of the eviction of a cache entry that has
 *   had a memory location in it modified, encoded as Log 2 of the number
 *   of words.
 *   A value of 0b0000 indicates that the CTR does not provide Cache 
 *   Write-back Granule information and either:
 *
 *   •    the architectural maximum of 512 words (2Kbytes) must be assumed
 *   •    the Cache Write-back Granule can be determined from maximum cache
 *        line size encoded in the Cache Size ID Registers.
 *
 *   Values greater than 0b1001 are reserved.
 *
 * ERG, bits[23:20]
 *
 *   Exclusives Reservation Granule. The maximum size of the reservation
 *   granule that has been implemented for the Load-Exclusive and 
 *   Store-Exclusive instructions, encoded as Log 2 of the number of words.
 *   For more information, see Tagging and the size of the tagged memory 
 *   block.
 *
 *   A value of 0b0000 indicates that the CTR does not provide Exclusives 
 *   Reservation Granule information and the architectural maximum of 512
 *   words (2Kbytes) must be assumed. Values greater than 0b1001 are reserved.
 *
 * DminLine, bits[19:16]
 *
 *   Log 2 of the number of words in the smallest cache line of all the
 *   data caches and unified caches that are controlled by the processor.
 *
 * L1Ip, bits[15:14]
 *
 *   Level 1 instruction cache policy. Indicates the indexing and tagging
 *   policy for the L1 instruction cache. Table shows the possible values
 *   for this field.
 *
 *   Table Level 1 instruction cache policy
 *   ----------+--------------------------------------------------
 *   L1lp bits | L1 instruction cache indexing and tagging policy
 *   ----------+--------------------------------------------------
 *   00        | Reserved
 *   ----------+--------------------------------------------------
 *   01        | ASID-tagged Virtual Index, Virtual Tag (AIVIVT)
 *   ----------+--------------------------------------------------
 *   10        | Virtual Index, Physical Tag (VIPT)
 *   ----------+--------------------------------------------------
 *   11        | Physical Index, Physical Tag (PIPT)
 *   ----------+--------------------------------------------------
 *
 * Bits[13:4]
 *
 *   RAZ
 *
 * IminLine, bits[3:0]
 *
 *   Log 2 of the number of words in the smallest cache line of all the
 *   instruction caches that are controlled by the processor.
 */

static int debug_CTR(void)
{
	unsigned long CTR;

	/* Read CTR into Rt */
	__asm__ volatile ("mrc p15, 0, %0, c0, c0, 1" : "=r" (CTR));

	/* Detect CTR type */
	if ((CTR >> 31) & 0x1) {
		printk("ARMv7 format.\n");
	} else {
		printk("ARM6 format.\n");
	}

	/* Cache Write-back Granule */
	printk("CWG: %#lx\n", (CTR >> 24) & 0xF);

	/* Exclusives Reservation Granule */
	printk("ERG: %#lx\n", (CTR >> 20) & 0xF);

	/* Smallest cache line of all the data caches and unified caches */
	printk("DminLine: %#lx\n", (CTR >> 16) & 0xF);

	/* Detect Level 1 instruction cache policy */
	if (((CTR >> 14) & 0x3) == 0x00) {
		printk("Level 1 instruction cache polity reserved.\n");
	} else if (((CTR >> 14) & 0x3) == 0x01) {
		printk("ASID-tagged Virtual Index, Virtual Tag (AIVIVT)\n");
	} else if (((CTR >> 14) & 0x3) == 0x02) {
		printk("Virtual Index, Physical Tag (VIPT)\n");
	} else {
		printk("Physical Index, Physical Tag (PIPT)\n");
	}

	/* Smallest cache line of all the instruction cache */
	printk("IminLine: %#lx\n", CTR & 0xF);

	return 0;
}
device_initcall(debug_CTR);
