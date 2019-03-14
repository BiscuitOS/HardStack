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

#ifdef CONFIG_DEBUG_MEMBLOCK_HELPER
int __init debug_memblock_helper(void)
{
	struct memblock_region *reg;
	phys_addr_t size;
	phys_addr_t addr;
	phys_addr_t limit;
	bool state;
	int nid;

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

	/* Obtain memblock.reserved total size */
	size = memblock_reserved_size();
	pr_info("Reserved Memory total size: %#x\n", size);

	/* Obtain Start physical address of DRAM */
	addr = memblock_start_of_DRAM();
	pr_info("Start address of DRAM:      %#x\n", addr);

	/* Obtain End of physical address of DRAM */
	addr = memblock_end_of_DRAM();
	pr_info("End address of DRAM:        %#x\n", addr);

	/* Check address is memblock.reserved */
	addr = 0x81000000; /* Assume address in memblock.reserved */
	state = memblock_is_reserved(addr);
	if (state)
		pr_info("Address: %#x in reserved.\n", addr);

	/* Check address in memblock.memory */
	addr = 0x62000000; /* Assume address in memblock.memory */
	state = memblock_is_memory(addr);
	if (state)
		pr_info("Address: %#x in memory.\n", addr);

	/* Check region in memblock.memory */
	addr = 0x62000000;
	size = 0x100000; /* Assume [0x62000000, 0x62100000] in memory */
	state = memblock_is_region_memory(addr, size);
	if (state)
		pr_info("Region: [%#x - %#x] in memblock.memory.\n",
				addr, addr + size);

	/* Check region in memblock.reserved */
	addr = 0x80000000;
	size = 0x100000; /* Assume [0x80000000, 0x80100000] in reserved */
	state = memblock_is_region_reserved(addr, size);
	if (state)
		pr_info("Region: [%#x - %#x] in memblock.reserved.\n",
				addr, addr + size);

	/* Obtain current limit for memblock */
	limit = memblock_get_current_limit();
	pr_info("MEMBLOCK current_limit: %#x\n", limit);

	/* Set up current_limit for MEMBLOCK */
	memblock_set_current_limit(limit);

	/* Check memblock regions is hotpluggable */
	state = memblock_is_hotpluggable(&memblock.memory.regions[0]);
	if (state)
		pr_info("MEMBLOCK memory.regions[0] is hotpluggable.\n");
	else
		pr_info("MEMBLOCK memory.regions[0] is not hotpluggable.\n");

	/* Check memblock regions is mirror */
	state = memblock_is_mirror(&memblock.memory.regions[0]);
	if (state)
		pr_info("MEMBLOCK memory.regions[0] is mirror.\n");
	else
		pr_info("MEMBLOCK memory.regions[0] is not mirror.\n");

	/* Check memblock regions is nomap */
	state = memblock_is_nomap(&memblock.memory.regions[0]);
	if (state)
		pr_info("MEMBLOCK memory.regions[0] is nomap.\n");
	else
		pr_info("MEMBLOCK memory.regions[0] is not nomap.\n");

	/* Check region nid information */
	nid = memblock_get_region_node(&memblock.memory.regions[0]);
	pr_info("MEMBLOCK memory.regions[0] nid: %#x\n", nid);
	/* Set up region nid */
	memblock_set_region_node(&memblock.memory.regions[0], nid);

	/* Obtian MEMBLOCK allocator direction */
	state = memblock_bottom_up();
	pr_info("MEMBLOCK direction: %s", state ? "bottom-up" : "top-down");
	/* Set up MEMBLOCK allocate direction */
	memblock_set_bottom_up(state);

	return 0;
}
#endif
