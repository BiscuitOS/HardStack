/*
 * memblock allocator
 *
 * (C) 2019.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MEMBLOCK
 *
 *                                         
 *                                         struct memblock_region
 *                       struct            +------+------+--------+------+
 *                       memblock_type     |      |      |        |      |
 *                       +----------+      | Reg0 | Reg1 | ...    | Regn |
 *                       |          |      |      |      |        |      |
 *                       | regions -|----->+------+------+--------+------+
 *                       | cnt      |      [memblock_memory_init_regions]
 *                       |          |
 * struct           o--->+----------+
 * memblock         |
 * +-----------+    |
 * |           |    |
 * | memory   -|----o
 * | reserved -|----o
 * |           |    |                      struct memblock_region
 * +-----------+    |    struct            +------+------+--------+------+
 *                  |    memblock_type     |      |      |        |      |
 *                  o--->+----------+      | Reg0 | Reg1 | ...    | Regn |
 *                       |          |      |      |      |        |      |
 *                       | regions -|----->+------+------+--------+------+
 *                       | cnt      |      [memblock_reserved_init_regions]
 *                       |          |
 *                       +----------+
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>

int bs_debug = 0;

#ifdef CONFIG_DEBUG___NEXT_RESERVED_MEM_REGION
/*
 * Mark memory as reserved on memblock.reserved regions.
 */
int debug___next_reserved_mem_region(void)
{
	struct memblock_region *reg;
	u64 idx;
	phys_addr_t start;
	phys_addr_t end;

	/*
	 * Emulate memory
	 *
	 *
	 *                    memblock.memory
	 * 0   | <--------------------------------------------> |
	 * +---+-------+--------+--------+--------+-------------+-----+
	 * |   |       |        |        |        |             |     |
	 * |   |       |        |        |        |             |     |
	 * |   |       |        |        |        |             |     |
	 * +---+-------+--------+--------+--------+-------------+-----+
	 *             | <----> |        | <----> |
	 *             Reserved 0        Reserved 1
	 *
	 * Memory Region:      [0x60000000, 0x80000000]
	 * Reserved Region 0:  [0x68000000, 0x69000000]
	 * Reserved Region 1:  [0x78000000, 0x7a000000]
	 */
	memblock_reserve(0x68000000, 0x1000000);
	memblock_reserve(0x78000000, 0x2000000);
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	idx = 0UL;
	__next_reserved_mem_region(&idx, &start, &end);
	pr_info("Next-Region: %#x - %#x\n", start, end);

	/* Clear debug case */
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;

	return 0;
}
#endif
