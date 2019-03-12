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

#ifdef CONFIG_DEBUG_MEMBLOCK_FREE
/*
 * Free an reserved region.
 */
int debug_memblock_free(void)
{
	struct memblock_region *reg;

	/*
	 * Emulate memory
	 *
	 *                   memblock.memory
	 *     | <--------------------------------------> |
	 * +---+----------------------+--------------+----+-------+
	 * |   |                      |              |    |       |
	 * |   |                      |              |    |       |
	 * |   |                      |              |    |       |
	 * +---+----------------------+--------------+----+-------+
	 *                            | <----------> |
	 *                            memblock.reserved
	 *
	 * Memory Region:   [0x60000000, 0x80000000]
	 * Reserved Region: [0x78000000, 0x7a000000]
	 * Free Region:     [0x78000000, 0x79000000]
	 */
	memblock_reserve(0x78000000, 0x2000000);
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	/*
	 * Processing free
	 *
	 *                   memblock.memory
	 *     | <--------------------------------------> |
	 * +---+--------------------------------+-----+----+-------+
	 * |   |                                |     |    |       |
	 * |   |                                |     |    |       |
	 * |   |                                |     |    |       |
	 * +---+--------------------------------+-----+----+-------+
	 *                                      | <-> |
	 *                                    Remain region
	 *
	 * Memory Region:   [0x60000000, 0x80000000]
	 * Reserved Region: [0x79000000, 0x7a000000]
	 */
	memblock_free(0x78000000, 0x1000000);
	pr_info("Free special region.\n");
	for_each_memblock(reserved, reg)
		pr_info("Region: %#x - %#x\n", reg->base,
				reg->base + reg->size);

	/* Clear debug case */
	memblock.reserved.cnt = 1;
	memblock.reserved.total_size = 0;

	return 0;
}
#endif
