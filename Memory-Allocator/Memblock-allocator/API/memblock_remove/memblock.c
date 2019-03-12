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

#ifdef CONFIG_DEBUG_MEMBLOCK_REMOVE
/*
 * Remove a special memory region.
 */
int debug_memblock_remove(void)
{
	struct memblock_region *reg;

	/*
	 * Emulate memory
	 *
	 *            System memory
	 *           | <----------> |
	 * +---------+--------------+--------+-------------+-----------+
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * +---------+--------------+--------+-------------+-----------+
	 *                                   | <---------> |
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 */
	memblock_add(0xb0000000, 0xd000000);
	/* Scan old memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	/*
	 * Remove region behind Pseudo memory
	 *
	 *
	 *            System memory                         Remove region
	 *           | <----------> |                         | <-> |
	 * +---------+--------------+--------+-------------+--+-----+--+
	 * |         |              |        |             |  |     |  |
	 * |         |              |        |             |  |     |  |
	 * |         |              |        |             |  |     |  |
	 * +---------+--------------+--------+-------------+--+-----+--+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xbe000000, 0xbe200000]
	 *
	 * Processing Remove
	 *
	 *            System memory                         
	 *           | <----------> |                         
	 * +---------+--------------+--------+-------------+-----------+
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * +---------+--------------+--------+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 */
	memblock_remove(0xbe000000, 0x200000);
	pr_info("Remove Behind Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region behind and conjuncte Pseudo memory
	 *
	 *
	 *            System memory                       Remove region
	 *           | <----------> |                      | <----> |
	 * +---------+--------------+--------+-------------+--------+--+
	 * |         |              |        |             |        |  |
	 * |         |              |        |             |        |  |
	 * |         |              |        |             |        |  |
	 * +---------+--------------+--------+-------------+--------+--+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xbd000000, 0xbd200000]
	 *
	 * Processing Remove
	 *
	 *            System memory                         
	 *           | <----------> |                         
	 * +---------+--------------+--------+-------------+-----------+
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * +---------+--------------+--------+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 */
	memblock_remove(0xbd000000, 0x200000);
	pr_info("Remove Behind and Conjunct Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region behind and cross Pseudo memory
	 *
	 *
	 *            System memory                    Remove region
	 *           | <----------> |                 | <---------> |
	 * +---------+--------------+--------+--------+----+--------+--+
	 * |         |              |        |        |    |        |  |
	 * |         |              |        |        |    |        |  |
	 * |         |              |        |        |    |        |  |
	 * +---------+--------------+--------+--------+----+--------+--+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xbc000000, 0xbe000000]
	 *
	 * Processing Remove
	 *
	 *            System memory                         
	 *           | <----------> |                         
	 * +---------+--------------+--------+--------+----------------+
	 * |         |              |        |        |                |
	 * |         |              |        |        |                |
	 * |         |              |        |        |                |
	 * +---------+--------------+--------+--------+----------------+
	 *                                   | <----> |  
	 *                                  Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbc000000]
	 */
	memblock_remove(0xbc000000, 0x2000000);
	pr_info("Remove Behind and Cross Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);
	
	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region behind and contain Pseudo memory
	 *
	 *
	 *            System memory                Remove region
	 *           | <----------> |        | <------------------> |
	 * +---------+--------------+--------+-------------+--------+--+
	 * |         |              |        |             |        |  |
	 * |         |              |        |             |        |  |
	 * |         |              |        |             |        |  |
	 * +---------+--------------+--------+-------------+--------+--+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xb0000000, 0xbe000000]
	 *
	 * Processing Remove
	 *
	 *            System memory                         
	 *           | <----------> |                         
	 * +---------+--------------+----------------------------------+
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * +---------+--------------+----------------------------------+
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 */
	memblock_remove(0xb0000000, 0xe000000);
	pr_info("Remove Behind and Contain Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region in Pseudo memory
	 *
	 *
	 *            System memory           Remove region
	 *           | <----------> |           | <-> |
	 * +---------+--------------+--------+--+-----+----+-----------+
	 * |         |              |        |  |     |    |           |
	 * |         |              |        |  |     |    |           |
	 * |         |              |        |  |     |    |           |
	 * +---------+--------------+--------+--+-----+----+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xb2000000, 0xb3000000]
	 *
	 * Processing Remove
	 *
	 *            System memory       Pseudo memory  
	 *           | <----------> |        |<>|
	 * +---------+--------------+--------+--+-----+----+-----------+
	 * |         |              |        |  |     |    |           |
	 * |         |              |        |  |     |    |           |
	 * |         |              |        |  |     |    |           |
	 * +---------+--------------+--------+--+-----+----+-----------+
	 *                                            | <> |
	 *                                        Pseudo memory  
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xb2000000]
	 * Pseudo Memory: [0xb3000000, 0xbd000000]
	 * 
	 */
	memblock_remove(0xb2000000, 0x1000000);
	pr_info("Remove in Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region equal to Pseudo memory
	 *
	 *
	 *            System memory           Remove region
	 *           | <----------> |        | <---------> |
	 * +---------+--------------+--------+-------------+-----------+
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * |         |              |        |             |           |
	 * +---------+--------------+--------+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xb0000000, 0xbd000000]
	 *
	 * Processing Remove
	 *
	 *            System memory      
	 *           | <----------> |        
	 * +---------+--------------+----------------------------------+
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * +---------+--------------+----------------------------------+
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 */
	memblock_remove(0xb0000000, 0xd000000);
	pr_info("Remove in Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region forward and contain Pseudo memory
	 *
	 *
	 *            System memory         Remove region
	 *           | <----------> |   | <--------------> |
	 * +---------+--------------+---+----+-------------+-----------+
	 * |         |              |   |    |             |           |
	 * |         |              |   |    |             |           |
	 * |         |              |   |    |             |           |
	 * +---------+--------------+---+----+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xaf000000, 0xbd000000]
	 *
	 * Processing Remove
	 *
	 *            System memory      
	 *           | <----------> |        
	 * +---------+--------------+----------------------------------+
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * |         |              |                                  |
	 * +---------+--------------+----------------------------------+
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 */
	memblock_remove(0xaf000000, 0xe000000);
	pr_info("Remove Forward and Contain Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region forward and cross Pseudo memory
	 *
	 *
	 *            System memory     Remove region
	 *           | <----------> |   | <-------> |
	 * +---------+--------------+---+----+------+------+-----------+
	 * |         |              |   |    |      |      |           |
	 * |         |              |   |    |      |      |           |
	 * |         |              |   |    |      |      |           |
	 * +---------+--------------+---+----+------+------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xaf000000, 0xb1000000]
	 *
	 * Processing Remove
	 *
	 *            System memory     
	 *           | <----------> |   
	 * +---------+--------------+---------------+------+-----------+
	 * |         |              |               |      |           |
	 * |         |              |               |      |           |
	 * |         |              |               |      |           |
	 * +---------+--------------+---------------+------+-----------+
	 *                                          | <--> |  
	 *                                        Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb1000000, 0xbd000000]
	 */
	memblock_remove(0xaf000000, 0x2000000);
	pr_info("Remove Forward and Cross Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region forward and continue Pseudo memory
	 *
	 *
	 *            System memory   Remove region
	 *           | <----------> |  | <-> |
	 * +---------+--------------+--+-----+-------------+-----------+
	 * |         |              |  |     |             |           |
	 * |         |              |  |     |             |           |
	 * |         |              |  |     |             |           |
	 * +---------+--------------+--+-----+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xaf000000, 0xb0000000]
	 *
	 * Processing Remove
	 *
	 *            System memory     
	 *           | <----------> |   
	 * +---------+--------------+--------+------------+-----------+
	 * |         |              |        |            |           |
	 * |         |              |        |            |           |
	 * |         |              |        |            |           |
	 * +---------+--------------+--------+------------+-----------+
	 *                                   | <--------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 */
	memblock_remove(0xaf000000, 0x1000000);
	pr_info("Remove Forward and Continue Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	memblock_add(0xb0000000, 0xd000000);
	/*
	 * Remove region forward and disconjunct Pseudo memory
	 *
	 *
	 *            System memory  Remove region
	 *           | <----------> |  |<->|
	 * +---------+--------------+--+---+-+-------------+-----------+
	 * |         |              |  |   | |             |           |
	 * |         |              |  |   | |             |           |
	 * |         |              |  |   | |             |           |
	 * +---------+--------------+--+---+-+-------------+-----------+
	 *                                   | <---------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 * Remove Region: [0xae000000, 0xaf000000]
	 *
	 * Processing Remove
	 *
	 *            System memory     
	 *           | <----------> |   
	 * +---------+--------------+--------+------------+-----------+
	 * |         |              |        |            |           |
	 * |         |              |        |            |           |
	 * |         |              |        |            |           |
	 * +---------+--------------+--------+------------+-----------+
	 *                                   | <--------> |  
	 *                                    Pseudo memory
	 *
	 * Real Memory:   [0x60000000, 0xa0000000]
	 * Pseudo Memory: [0xb0000000, 0xbd000000]
	 */
	memblock_remove(0xae000000, 0x1000000);
	pr_info("Remove Forward and Disconjunct Pseudo Region:\n");
	/* Dump all memory region */
	for_each_memblock(memory, reg)
		pr_info("Memory-Region: %#x - %#x\n", reg->base, 
				reg->base + reg->size);

	/* Clear debug case */
	memblock.memory.cnt = 1;
	memblock.memory.total_size = memblock.memory.regions[0].size;
	memblock.current_limit = memblock.memory.regions[0].base + 
			memblock.memory.regions[0].size;

	return 0;
}
#endif
