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
 * CLIDR, Cache Level ID Register, VMSA
 *
 * **Purpose**
 *
 *   The CLIDR identifies:
 *
 *   •    the type of cache, or caches, implemented at each level, up to
 *        a maximum of seven levels.
 *
 *   •    the Level of Coherence (LoC) and Level of Unification (LoU) for
 *        the cache hierarchy.
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
 *   is Common. A 32-bit RO register with an IMPLEMENTATION DEFINED value. 
 *   See also Reset behavior of CP14 and CP15 registers on page B3-1450.
 *
 *
 * 32  30   27    24    21    18    15    12    9     6     3     0
 * +-+-+----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * | | |    |     |     |     |     |     |     |     |     |     |   
 * |0|0|LoUU| Loc |LoUIS|     |     |     |     |     |     |     |   
 * | | |    |     |     |     |     |     |     |     |     |     |   
 * +-+-+----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                         A     A     A     A     A     A     A
 *                         |     |     |     |     |     |     |
 *                         |     |     |     |     |     |     o---- Ctype1
 *                         |     |     |     |     |     o---------- Ctype2
 *                         |     |     |     |     o---------------- Ctype3
 *                         |     |     |     o---------------------- Ctype4
 *                         |     |     o---------------------------- Ctype5
 *                         |     o---------------------------------- Ctype6
 *                         o---------------------------------------- Ctype7
 *
 *
 *
 * Bits[31:30]
 *
 *   Reserved, UNK
 *
 * LoUU, bits[29:27]
 *
 *   Level of Unification Uniprocessor for the cache hierarchy, see 
 *   Terminology for Clean, Invalidate, and Clean and Invalidate operations.
 *
 * LoC, bits[26:24]
 *
 *   Level of Coherence for the cache hierarchy, see Terminology for Clean,
 *   Invalidate, and Clean and Invalidate operations.
 *
 * LoUIS, bits[23:21]
 *
 *   Level of Unification Inner Shareable for the cache hierarchy, see 
 *   Terminology for Clean, Invalidate, and Clean and Invalidate operations.
 *   In an implementation that does not include the Multiprocessing 
 *   Extensions, this field is RAZ.
 *
 * Ctypen, bits[3(n - 1) + 2:3(n - 1)], for n = 1 to 7
 *
 *   Cache Type fields. Indicate the type of cache implemented at each level,
 *   from Level 1 up to a maximum of seven levels of cache hierarchy. The 
 *   Level 1 cache field, Ctype1, is bits[2:0], see register diagram. Table 
 *   shows the possible values for each Ctypen field.
 *
 *   Table Ctype n bit values
 *   --------------+---------------------------------------------
 *   Ctype n value | Meaning, cache implemented at this level
 *   --------------+---------------------------------------------
 *   000           | No cache
 *   --------------+---------------------------------------------
 *   001           | Instruction cache only
 *   --------------+---------------------------------------------
 *   010           | Data cache only
 *   --------------+---------------------------------------------
 *   011           | Separate instruction and data caches
 *   --------------+---------------------------------------------
 *   100           | Unified cache
 *   --------------+---------------------------------------------
 *   101,11X       | Reserved
 *   --------------+---------------------------------------------
 *
 *   If software reads the Cache Type fields from Ctype1 upwards, once it 
 *   has seen a value of 0b000 , no caches exist at further-out levels of
 *   the hierarchy. So, for example, if Ctype3 is the first Cache Type
 *   field with a value of 0b000 , the values of Ctype4 to Ctype7 must be
 *   ignored. 
 *
 * The CLIDR describes only the caches that are under the control of the 
 * processor.
 */
static void cache_ctype(int type)
{
	switch (type) {
	case 0x00:
		printk("Cache: No cache\n");
		break;
	case 0x01:
		printk("Cache: Instruction cache only.\n");
		break;
	case 0x02:
		printk("Cache: Data cache only.\n");
		break;
	case 0x03:
		printk("Cache: Separate instruction and data caches.\n");
		break;
	case 0x04:
		printk("Cache: Unified cache.\n");
		break;
	default:
		printk("Reserved\n");
		break;
	}
}

static int debug_CLIDR(void)
{
	unsigned long CLIDR;
	unsigned int  index;
	int i;

	__asm__ volatile ("mrc p15, 1, %0, c0, c0, 1" : "=r" (CLIDR));

	/* Level of Unification Uniprocessor cache: LoUU */
	index = (CLIDR >> 27) & 0x7;
	printk("Level of LoUU: Ctype%d\n", index);
	index = (index - 1) * 3;
	cache_ctype((CLIDR >> index) & 0x7);

	/* Level of Coherence cache: LoC */
	index = (CLIDR >> 24) & 0x7;
	printk("Level of LoC: Ctype%d\n", index);
	index = (index - 1) * 3;
	cache_ctype((CLIDR >> index) & 0x7);

	/* Level of Unification Inner Shareable cache: LoUIS */
	index = (CLIDR >> 21) & 0x7;
	printk("Level of LoUIS: ctype%d\n", index);
	index = (index - 1) * 3;
	cache_ctype((CLIDR >> index)  & 0x7);

	/* ctype */
	for (i = 1; i < 8; i++) {
		index = (i - 1) * 3;
		printk("ctype%d ", i);
		cache_ctype((CLIDR >> index) & 0x7);
	}

	return 0;
}
device_initcall(debug_CLIDR);
