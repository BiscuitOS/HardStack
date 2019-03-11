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

#ifdef CONFIG_DEBUG_MEMBLOCK_ADD_RANGE
/*
 * Mark memory as reserved on memblock.reserved regions.
 */
int debug_memblock_add_range(void)
{
	struct memblock_region *reg;

	/* Scan old reserved region */
	for_each_memblock(reserved, reg)
		pr_info("Reserved-Region: %#x -- %#x\n", reg->base, 
		 	reg->base + reg->size);
	for_each_memblock(memory, reg)
		pr_info("Memory-Region:   %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	/* Reserved memblock region is empty and insert a new region 
	 *
	 * memblock.reserved--->+--------+
	 *                      |        |
	 *                      | Empty- |
	 *                      |        |
	 *                      +--------+
	 */
	memblock_add_range(&memblock.reserved, 
			0x60000000, 0x200000, MAX_NUMNODES, 0);
	pr_info("Scan region:\n");
	for_each_memblock(reserved, reg)
		pr_info("Reserved-Region: %#x - %#x\n", reg->base, 
			reg->base + reg->size);

	/* Clear debug data from memblock.reserved */
	for_each_memblock(reserved, reg) {
		reg->base = 0;
		reg->size = 0;
	}
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;

	return 0;
}
#endif
