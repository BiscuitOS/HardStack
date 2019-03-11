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

#ifdef CONFIG_DEBUG_MEMBLOCK_REMOVE
/*
 * Mark memory as reserved on memblock.reserved regions.
 */
int debug_memblock_remove(void)
{
	struct memblock_region *reg;

	/*
	 * Emulate memory
	 *
	 *            System memory
	 *           | <----------> |
	 * +---------+--------------+--------+---------+---------------+
	 * |         |              |        |         |               |
	 * |         |              |        |         |               |
	 * |         |              |        |         |               |
	 * +---------+--------------+--------+---------+---------------+
	 *                                   | <-----> |
	 *                                   pseudo memory
	 *
	 * Real Memory:   [0x6000000, 0xa0000000]
	 * Pseudo Memory: [0xb000000, 0xb0200000]
	 */
	memblock_add(0xb0000000, 0x200000);
	/* Scan old memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	/* 
	 * Remove pseudo memory [0xb0000000, 0xb0200000] 
	 *
	 *            System memory
	 *           | <----------> |
	 * +---------+--------------+----------------------------------+
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * +---------+--------------+----------------------------------+
	 *
	 * Real Memory: [0x60000000, 0xa0000000]
	 */
	memblock_remove(0xb0000000, 0x200000);
	/* Traverse memblock.memory regions */
	for_each_memblock(memory, reg)
		pr_info("Scan-Region:   %#x - %#x\n", reg->base,
				reg->base + reg->size);


	/* Clear debug case */
	memblock.memory.cnt = 1;
	memblock.memory.total_size -= 0x200000;
	memblock.current_limit = memblock.memory.regions[0].base + 
			memblock.memory.regions[0].size;

	return 0;
}
#endif
