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

#ifdef CONFIG_DEBUG_MEMBLOCK_GET_REGION_NODE
int __init debug_memblock_get_region_node(void)
{
	int nid;

	/* Check region nid information */
	nid = memblock_get_region_node(&memblock.memory.regions[0]);
	pr_info("MEMBLOCK memory.regions[0] nid: %#x\n", nid);

	return 0;
}
#endif
