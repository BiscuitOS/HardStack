/*
 * memblock allocator
 *
 * (C) 2019.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

int bs_debug = 0;

#ifdef CONFIG_DEBUG_FOR_EACH_MEM_RANGE_REV
int debug_for_each_mem_range_rev(void)
{
	enum memblock_flags flags = choose_memblock_flags();
	phys_addr_t start;
	phys_addr_t end;
	u64 idx; /* (memory.cnt << 32) | (reserved.cnt) */

	/*                
	 * Memory Maps:
	 *
	 *                     Reserved 0            Reserved 1
	 *                   | <------> |          | <------> |
	 * +-----------------+----------+----------+----------+------+
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * +-----------------+----------+----------+----------+------+
	 *                              | <------> |
	 *                               Found area
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
	 * Reserved 1: [0x64300000, 0x64400000]
	 */
	memblock_reserve(0x64000000, 0x100000);
	memblock_reserve(0x64300000, 0x100000);

	for_each_mem_range_rev(idx, &memblock.memory, &memblock.reserved, 
			NUMA_NO_NODE, flags, &start, &end, NULL)
		pr_info("Region: [%#x - %#x]\n", start, end);

	/* Clear rservation for debug */
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;
	
	return 0;
}
#endif
