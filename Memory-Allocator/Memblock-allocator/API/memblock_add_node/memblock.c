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

#ifdef CONFIG_DEBUG_MEMBLOCK_ADD_NODE
int debug_memblock_add_node(void)
{
	struct memblock_region *reg;

	/* Scan old memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region:   %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add_node(0x90000000, 0x200000, MAX_NUMNODES);
	pr_info("Scan region:\n");
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
			reg->base + reg->size);

	/* Clear debug case */
	memblock.memory.cnt = 1;
	memblock.memory.total_size -= 0x200000;
	memblock.current_limit = memblock.memory.regions[0].base + 
			memblock.memory.regions[0].size;

	return 0;
}
#endif
