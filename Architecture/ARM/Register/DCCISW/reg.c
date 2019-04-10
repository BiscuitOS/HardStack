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
 * DCCISW: Clear and invlidate data cache line by set/way
 *
 *   The DCCISW register are write-only. Reads return 0. This operation
 *   is part of the Cache maintenance operations functional group.
 *
 * The bit assignments are:
 *
 * 32    29                               14              5          1 0
 * +-----+--------------------------------+---------------+----------+-+
 * |     |                                |               |          | |
 * | Way |            Reserved            |      Set      | Reserved |0|
 * |     |                                |               |          | |
 * +-----+--------------------------------+---------------+----------+-+
 *
 * Way[31:30]
 * 
 *   Way that operation applies to. For the data cache, values 0,1,2, and 3
 *   are supported. Write-only.
 *
 * Reserved[29:14]
 * 
 * Set[13:5]
 *
 *   Set/Index that operation applies to. The number of indices in a cache
 *   depends on the configured cache size. When this is less than the maximum,
 *   use the LSB of this field. The number of sets in the cache can be 
 *   determined by reading the Cache Size ID Register.
 *
 * Reserved[4:1]
 *
 * [0] Always reads as zero.
 */

static int debug_DCCISW(void)
{
	unsigned long DCCISW = 0x0;

	/* Clean and invalidate data cache by se/way */
	__asm__ volatile ("mcr p15, 0, %0, c7, c14, 2" :: "r" (DCCISW));

	return 0;
}
device_initcall(debug_DCCISW);
