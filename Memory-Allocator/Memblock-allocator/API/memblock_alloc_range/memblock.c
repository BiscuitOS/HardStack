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

#ifdef CONFIG_DEBUG_MEMBLOCK_ALLOC_RANGE
int __init debug_memblock_alloc_range(void)
{
	enum memblock_flags flags = choose_memblock_flags();
	struct memblock_region *reg;
	phys_addr_t addr;

	/*
	 * Memory Map
	 *
	 *                  memblock.memory
	 * 0    | <---------------------------------> |
	 * +----+-------------------------------------+-----------+
	 * |    |                                     |           |
	 * |    |                                     |           |
	 * |    |                                     |           |
	 * +----+-------------------------------------+-----------+
	 *                                            
	 * Memory Region: [0x60000000, 0xa0000000]
	 */
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
			reg->base + reg->size);

	/*
	 * Memory Map
	 *
	 *                  memblock.memory
	 * 0    | <---------------------------------> |
	 * +----+------------------------------+------+-----------+
	 * |    |                              |      |           |
	 * |    |                              |      |           |
	 * |    |                              |      |           |
	 * +----+------------------------------+------+-----------+
	 *                                     | <--> |
	 *                                      region
	 *                                            
	 * Memory Region: [0x60000000, 0xa0000000]
	 * Found Region:  [0x9ff00000, 0xa0000000]
	 */
	addr = memblock_alloc_range(0x100000, 0x1000,
			MEMBLOCK_LOW_LIMIT, ARCH_LOW_ADDRESS_LIMIT, flags);
	pr_info("Find address: %#x\n", addr);	

	/* Dump all reserved region */
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	return 0;
}
#endif
