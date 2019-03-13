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

#ifdef CONFIG_DEBUG_MEMBLOCK_PHYS_MEM_SIZE
int __init debug_memblock_phys_mem_size(void)
{
	struct memblock_region *reg;
	phys_addr_t size;

	/*
	 * Emulate memory
	 *
	 *                      memblock.memory
	 * 0     | <----------------------------------------> |
	 * +-----+---------+-------+----------+-------+-------+----+
	 * |     |         |       |          |       |       |    |
	 * |     |         |       |          |       |       |    |
	 * |     |         |       |          |       |       |    |
	 * +-----+---------+-------+----------+-------+-------+----+
	 *                 | <---> |          | <---> |
	 *                 Reserved 0         Reserved 1
	 *
	 * Memroy Region:   [0x60000000, 0xa0000000]
	 * Reserved Region: [0x80000000, 0x8d000000]
	 * Reserved Region: [0x90000000, 0x92000000]
	 */
	memblock_reserve(0x80000000, 0xd000000);
	memblock_reserve(0x90000000, 0x2000000);
	pr_info("Memory Regions:\n");
	for_each_memblock(memory, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
					reg->base + reg->size);
	pr_info("Reserved Regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
					reg->base + reg->size);

	/* Obtain memblock.memory total size */
	size = memblock_phys_mem_size();
	pr_info("Phyiscal Memory total size: %#x\n", size);

	return 0;
}
#endif
