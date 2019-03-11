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

#include <asm/sections.h>

int bs_debug = 0;

#ifdef CONFIG_DEBUG_MEMBLOCK_FIND_IN_RANGE_NODE
/*
 * Mark memory as reserved on memblock.reserved regions.
 */
int debug_memblock_find_in_range_node(void)
{
	enum memblock_flags flags = choose_memblock_flags();
	struct memblock_region *reg;
	phys_addr_t kernel_end;
	phys_addr_t addr;
	bool bottom_up;

	/*                
	 * Memory Maps:
	 *
	 *                     Reserved 0            Reserved 1
	 *          _end     | <------> |          | <------> |
	 * +--+-----+--------+----------+----------+----------+------+
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * +--+-----+--------+----------+----------+----------+------+
	 *    | <-> |
	 *    Kernel
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
	 * Reserved 1: [0x64300000, 0x64400000]
	 */
	memblock_reserve(0x64000000, 0x100000);
	memblock_reserve(0x64300000, 0x100000);
	kernel_end = __pa_symbol(_end);
	bottom_up = memblock_bottom_up();

	for_each_memblock(memory, reg)
		pr_info("Memory-Region:   %#x - %#x\n", reg->base, 
				reg->base + reg->size);
	for_each_memblock(reserved, reg)
		pr_info("Reserved-Region: %#x - %#x\n", reg->base,
				reg->base + reg->size);
	pr_info("Kernel End addr: %#x\n", kernel_end);

	/*
	 * Top-down allocate:
	 *
	 *                     Reserved 0            Reserved 1
	 *          _end     | <------> |          | <------> |
	 * +--+-----+--------+----------+----------+----------+------+
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * +--+-----+--------+----------+----------+----------+------+
	 *    | <-> |                                            <---|
	 *    Kernel
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
         * Reserved 1: [0x64300000, 0x64400000]
	 *
	 * Find free are in given range and node, memblock will allocate 
	 * memory in Top-down direction.
	 */
	memblock_set_bottom_up(false);
	addr = (phys_addr_t)memblock_find_in_range_node(0x100000, 0x1000, 
			MEMBLOCK_LOW_LIMIT, ARCH_LOW_ADDRESS_LIMIT,
			NUMA_NO_NODE, flags);
	pr_info("Top-down Addre:  %#x\n", addr);

	/*
	 * Bottom-up allocate:
	 *
	 *                     Reserved 0            Reserved 1
	 *          _end     | <------> |          | <------> |
	 * +--+-----+--------+----------+----------+----------+------+
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * +--+-----+--------+----------+----------+----------+------+
	 *    | <-> |---->
	 *    Kernel
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
         * Reserved 1: [0x64300000, 0x64400000]
	 *
	 * Find free are in given range and node, memblock will allocate 
	 * memory in bottom-up direction.
	 */
	memblock_set_bottom_up(true);
	addr = (phys_addr_t)memblock_find_in_range_node(0x100000, 0x1000, 
			MEMBLOCK_LOW_LIMIT, ARCH_LOW_ADDRESS_LIMIT,
			NUMA_NO_NODE, flags);
	pr_info("Bottom-up Addr:  %#x\n", addr);

	/*
	 * Special area allocate:
	 *
	 *                     Reserved 0            Reserved 1
	 *          _end     | <------> |          | <------> |
	 * +--+-----+--------+----------+----------+----------+------+
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * |  |     |        |          |          |          |      |
	 * +--+-----+--------+----------+----------+----------+------+
	 *    | <-> |                   |---->
	 *    Kernel
	 *
	 * Reserved 0: [0x64000000, 0x64100000]
         * Reserved 1: [0x64300000, 0x64400000]
	 *
	 * Find free are in given range and node, memblock will allocate 
	 * memory in bottom-up from special address.
	 */
	memblock_set_bottom_up(true);
	addr = (phys_addr_t)memblock_find_in_range_node(0x100000, 0x1000, 
			0x64100000, 0x64300000, NUMA_NO_NODE, flags);
	pr_info("Speical Address: %#x\n", addr);

	/* Clear debug case */
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;
	memblock_set_bottom_up(bottom_up);

	return 0;
}
#endif
