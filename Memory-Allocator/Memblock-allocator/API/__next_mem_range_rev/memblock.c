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

#ifdef CONFIG_DEBUG__NEXT_MEM_RANGE_REV
int debug__next_mem_range_rev(void)
{
	enum memblock_flags flags = choose_memblock_flags();
	struct memblock_region *reg;
	int cnt = 0;
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

	/*
	 * Found a valid memory area from tail of reserved memory
	 * Last reserved area: [0x64300000, 0x74400000]
	 *
	 *                     Reserved 0            Reserved 1
	 *                   | <------> |          | <------> |
	 * +-----------------+----------+----------+----------+------+
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * +-----------------+----------+----------+----------+------+
	 *                                                    | <--> |
	 *                                                 Searching area
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
	 * Reserved 1: [0x64300000, 0x64400000]
	 */
	idx = (u64)ULLONG_MAX;
	__next_mem_range_rev(&idx, NUMA_NO_NODE, flags, 
		&memblock.memory, &memblock.reserved, &start, &end, NULL);
	pr_info("Valid memory behine last reserved:  [%#x - %#x]\n",
			start, end);

	/*
	 * Found a valid memory area from head of memory.
	 * 
	 *
	 *                     Reserved 0            Reserved 1
	 *                   | <------> |          | <------> |
	 * +-----------------+----------+----------+----------+------+
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * +-----------------+----------+----------+----------+------+
	 * | <-------------> |
	 *   Searching Area
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
	 * Reserved 1: [0x64300000, 0x64400000]
	 */
	idx = (u64)ULLONG_MAX & ((u64)0 << 32);
	__next_mem_range_rev(&idx, NUMA_NO_NODE, flags,
		&memblock.memory, &memblock.reserved, &start, &end, NULL);
	pr_info("Valid memory from head of memory:   [%#x - %#x]\n", 
			start, end);

	/*
	 * Found a valid memory area behind special reserved area.
	 * Special reserved area: [0x64000000, 0x64100000]
	 *
	 *                     Reserved 0            Reserved 1
	 *                   | <------> |          | <------> |
	 * +-----------------+----------+----------+----------+------+
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * +-----------------+----------+----------+----------+------+
	 *                              | <------> |
	 *                             Searching area
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
	 * Reserved 1: [0x64300000, 0x64400000]
	 */
	for_each_memblock(reserved, reg) {
		if (reg->base == 0x64000000)
			break;
		else
			cnt++;
	}
	idx = (u64)ULLONG_MAX & ((u64)++cnt << 32);
	__next_mem_range_rev(&idx, NUMA_NO_NODE, flags,
		&memblock.memory, &memblock.reserved, &start, &end, NULL);
	pr_info("Valid memory behine special region: [%#x - %#x]\n", 
			start, end);

	/*
	 * Found a valid memory area behind special index for 
	 * reservation area. e.g. index = 1
	 *
	 *                     Reserved 0            Reserved 1
	 *                   | <------> |          | <------> |
	 * +-----------------+----------+----------+----------+------+
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * |                 |          |          |          |      |
	 * +-----------------+----------+----------+----------+------+
	 *                                                    | <--> |
	 *                                                 Searching area
	 *
         * Memory:     [0x60000000, 0x64000000] index = 0
         * Reserved 0: [0x64000000, 0x64100000] index = 1
         * Reserved 1: [0x64300000, 0x64400000] index = 2
	 */
	idx = (u64)ULLONG_MAX & ((u64)1 << 32);
	__next_mem_range_rev(&idx, NUMA_NO_NODE, flags,
		&memblock.memory, &memblock.reserved, &start, &end, NULL);
	pr_info("Valid memory behind special index:  [%#x - %#x]\n", 
			start, end);


	/* Clear rservation for debug */
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;
	
	return 0;
}
#endif
