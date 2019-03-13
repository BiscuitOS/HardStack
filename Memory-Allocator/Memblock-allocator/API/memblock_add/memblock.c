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

#ifdef CONFIG_DEBUG_MEMBLOCK_ADD
int debug_memblock_add(void)
{
	struct memblock_region *reg;

	/*
	 * Emulate memory
	 *
	 *        System memory
	 *     | <-------------> |
	 * +---+-----------------+--------+-------------+------------+
	 * |   |                 |        |             |            |
	 * |   |                 |        |             |            |
	 * |   |                 |        |             |            |
	 * +---+-----------------+--------+-------------+------------+
	 *                                | <---------> |
	 *                                 Pseudo memory
	 * 
	 * System Memory: [0x60000000, 0xa0000000]
	 * Pseduo Memory: [0xb0000000, 0xc0000000]
	 *
	 */
	for_each_memblock(memory, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
			reg->base + reg->size);

	/* Add pseudo memory */
	pr_info("Add pseudo memory\n");
	memblock_add(0xb0000000, 0x10000000);
	for_each_memblock(memory, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
			reg->base + reg->size);

	/* Clear debug case */
	memblock.memory.cnt = 1;
	memblock.memory.total_size = memblock.memory.regions[0].size;
	memblock.current_limit = memblock.memory.regions[0].base
				+ memblock.memory.regions[0].size;

	return 0;
}
#endif
