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

#ifdef CONFIG_DEBUG_MEMBLOCK_RESERVE
/*
 * Mark memory as reserved on memblock.reserved regions.
 */
int debug_memblock_reserve(void)
{
	struct memblock_region *reg;

	/* Scan old reserved region */
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base, 
		 	reg->base + reg->size);

	/* Reserved memblock region is empty and insert a new region 
	 *
	 * memblock.reserved--->+--------+
	 *                      |        |
	 *                      | Empty- |
	 *                      |        |
	 *                      +--------+
	 */
	memblock_reserve(0x60000000, 0x200000);
	pr_info("Scan first region:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base, 
			reg->base + reg->size);

	/*
	 * Insert a new region which behine and disjunct an exist region:
	 *
	 * memblock.reserved:
	 *
	 * rbase                rend         base                  end
	 * +--------------------+            +---------------------+
	 * |                    |            |                     |
	 * |   Exist regions    |            |      new region     |
	 * |                    |            |                     |
	 * +--------------------+            +---------------------+
	 *
	 * 1) rend < base
	 * 2) rbase: 0x60000000
	 *    rend:  0x60200000
	 *    base:  0x62000000
	 *    end:   0x62200000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                rend         rbase                 rend
	 * +--------------------+            +---------------------+
	 * |                    |            |                     |
	 * |   Exist regions    |            |    Exist regions    |
	 * |                    |            |                     |
	 * +--------------------+            +---------------------+
	 *
	 * 1) rbase: 0x60000000
	 *    rend:  0x60200000
	 *    base:  0x62000000
	 *    end:   0x62200000
	 * 
	 */
	memblock_reserve(0x62000000, 0x200000);
	pr_info("Scan behine and disjunct region:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base, 
			reg->base + reg->size);

	/*
	 * Insert a new region which behine and conjunct an exist region:
	 *
	 * memblock.reserved:
	 *                      base
	 * rbase                rend                     end
	 * +--------------------+------------------------+
	 * |                    |                        |
	 * |   Exist regions    |       new region       |
	 * |                    |                        |
	 * +--------------------+------------------------+
	 *
	 * 1) base == rend
	 * 2) rbase: 0x62000000
	 *    rend:  0x62200000
	 *    base:  0x62200000
	 *    end:   0x62400000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                         rend
	 * +---------------------------------------------+
	 * |                                             |
	 * |   Exist regions                             |
	 * |                                             |
	 * +---------------------------------------------+
	 *
	 * 1) rbase: 0x62000000
	 *    rend:  0x62400000
	 */
	memblock_reserve(0x62200000, 0x200000);
	pr_info("Scan behine and conjunct region:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region that part of new region contains by exist 
	 * regions and part of new region behine exist regions:
	 *
	 * memblock.reserved:
	 *
	 * rbase                     rend                   
	 * | <---------------------> |
	 * +----------------+--------+----------------------+
	 * |                |        |                      |
	 * | Exist regions  |        |     new region       |
	 * |                |        |                      |
	 * +----------------+--------+----------------------+
	 *                  | <---------------------------> |
	 *                  base                            end
	 *
	 * 1) base > rebase
	 * 2) end  > rend
	 * 3) base > rend
	 * 4) rbase: 0x62000000
	 *    rend:  0x62400000
	 *    base:  0x62300000
	 *    end:   0x62500000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                            rend
	 * +------------------------------------------------+
	 * |                                                |
	 * | Exist regions                                  |
	 * |                                                |
	 * +------------------------------------------------+
	 *
	 * 1) rbase: 0x62000000
	 *    rend:  0x62500000
	 */
	memblock_reserve(0x62300000, 0x200000);
	pr_info("Scan behine but contain regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region that contain an exist regions which base 
	 * address is equal to new region. The end address of new region
	 * is big than exist regions.
	 *
	 * memblock.reserved:
	 *
	 * rbase:             rend
	 * | <--------------> |
	 * +------------------+-----------------------------+
	 * |                  |                             |
	 * |  Exist regions   |        new region           |
	 * |                  |                             |
	 * +------------------+-----------------------------+
	 * | <--------------------------------------------> |
	 * base                                             end
	 *
	 * 1) base == rbase
	 * 2) rend < end
	 * 3) rbase: 0x62000000
	 *    rend:  0x62500000
	 *    base:  0x62000000
	 *    end:   0x62600000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                            rend
	 * +------------------------------------------------+
	 * |                                                |
	 * |  Exist regions                                 |
	 * |                                                |
	 * +------------------------------------------------+
	 *
	 * 1) rbase: 0x62000000
	 *    rend:  0x62600000
	 *
	 */
	memblock_reserve(0x62000000, 0x600000);
	pr_info("Scan contain but equal regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region that contain whole exist regions.
	 *
	 * memblock.reserved:
	 *
	 * base             New regions                     end
	 * | <--------------------------------------------> |
	 * +-----------+--------------------+---------------+
	 * |           |                    |               |
	 * |           |   Exist regions    |               |
	 * |           |                    |               |
	 * +-----------+--------------------+---------------+
	 *             | <----------------> |
	 *             rbase                rend
	 *
	 * 1) base < rbase
	 * 2) rend < end
	 * 3) rbase: 0x62000000
	 *    rend:  0x62600000
	 *    base:  0x61000000
	 *    end:   0x63000000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                            rend
	 * +------------------------------------------------+
	 * |                                                |
	 * |              Exist Regions                     |
	 * |                                                |
	 * +------------------------------------------------+
	 *
	 * 1) rbase: 0x61000000
	 *    rend:  0x63000000
	 */
	memblock_reserve(0x61000000, 0x2000000);
	pr_info("Scan region which contain exist one:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base, 
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved regions that new region
	 * contain by exist regions, but the base address of new regions is 
	 * big than exist regions, and end address of new regions is equal
	 * to exist regions.
	 *
	 * memblock.reserved:
	 * 
	 * rbase           Exist regions                   rend
	 * | <-------------------------------------------> |
	 * +-------------------+---------------------------+
	 * |                   |                           |
	 * |                   |      New region           |
	 * |                   |                           |
	 * +-------------------+---------------------------+
	 *                     | <-----------------------> |
	 *                     base                        end
	 *
	 * 1) end == rend
	 * 2) rbase < base
	 * 3) rbase: 0x61000000
	 *    rend:  0x63000000
         *    base:  0x62000000
	 *    end:   0x63000000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                           rend
	 * +-----------------------------------------------+
	 * |                                               |
	 * |  Exist regions                                |
	 * |                                               |
	 * +-----------------------------------------------+
	 *
	 * 1) rbase: 0x61000000
	 *    rend:  0x63000000
	 */
	memblock_reserve(0x62000000, 0x1000000);
	pr_info("Scan region which contain new one:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved regions that exist region
	 * contains new region and the base address of exist region is equal
	 * to new, but the end address of exist is big than new.
	 *
	 * memblock.reserved:
	 *
	 * rbase              Exist regions               rend
	 * | <------------------------------------------> |
	 * +----------------+-----------------------------+
	 * |                |                             |
	 * |   New region   |                             |
	 * |                |                             |
	 * +----------------+-----------------------------+
	 * | <------------> |
	 * base             end
	 *
	 * 1) base == rbase
	 * 2) end < rend
	 * 3) rbase: 0x61000000
	 *    rend:  0x63000000
	 *    base:  0x61000000
	 *    end:   0x62000000
	 * 
	 * Processing: Insert/Merge
	 *
	 * rbase                                          rend
	 * +----------------------------------------------+
	 * |                                              |
	 * |  Exist Regions                               |
	 * |                                              |
	 * +----------------------------------------------+
	 *
	 * 1) rbase: 0x61000000
	 *    rend:  0x63000000
	 */
	memblock_reserve(0x61000000, 0x1000000);
	pr_info("Scan region which contain and head of regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base, 
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved and new region is
	 * same with exist regions.
	 * 
	 * memblock.reserved:
	 *
	 * rbase            Exist Regions                 rend
	 * | <------------------------------------------> |
	 * +----------------------------------------------+
	 * |                                              |
	 * |                                              |
	 * |                                              |
	 * +----------------------------------------------+
	 * | <------------------------------------------> |
	 * base             New region                    end
	 *
	 * 1) rbase = base
	 * 2) rend  = end
	 * 3) rbase: 0x61000000
	 *    rend:  0x63000000
	 *    base:  0x61000000
	 *    rend:  0x63000000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                          rend
	 * +----------------------------------------------+
	 * |                                              |
	 * | Exist regions                                |
	 * |                                              |
	 * +----------------------------------------------+
	 * 
	 * 1) rbase: 0x61000000
	 *    rend:  0x63000000
	 */
	memblock_reserve(0x61000000, 0x2000000);
	pr_info("Scan equal region:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved regions. The base address
	 * of new region is in the front of base address of exist regions,
	 * but end address of new region is big then exist.
	 *
	 * memblock.reserved
	 *
	 *                 rbase     Exist regions        rend
	 *                 | <--------------------------> |
	 * +---------------+--------+---------------------+
	 * |               |        |                     |
	 * |               |        |                     |
	 * |               |        |                     |
	 * +---------------+--------+---------------------+
	 * | <--------------------> |
	 * base   New region        end
	 *
	 * 1) rbase > base
	 * 2) rbase < end
	 * 3) end < rend
	 * 4) rbase: 0x61000000
	 *    rend:  0x63000000
	 *    base:  0x60f00000
	 *    ene:   0x61100000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                          rend
	 * +----------------------------------------------+
	 * |                                              |
	 * | Exist regions                                |
	 * |                                              |
	 * +----------------------------------------------+
	 *
	 * 1) rbase: 0x60f00000
	 *    rend:  0x63000000
	 */
	memblock_reserve(0x60f00000, 0x200000);
	pr_info("Scan forware and contain regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved regions, and new region
	 * is in front of exist regions and conjunct with exist regions.
	 *
	 * memblock.reserved:
	 *
	 *                        rbase                      rend
	 *                        | <----------------------> |
	 * +----------------------+--------------------------+
	 * |                      |                          |
	 * | New region           | Exist regions            |
	 * |                      |                          |
	 * +----------------------+--------------------------+
	 * | <------------------> |
	 * base                   end
	 *
	 * 1) end == rbase
	 * 2) rbase: 0x60f00000
	 *    rend:  0x63000000
	 *    base:  0x60e00000
	 *    end:   0x60f00000
	 *
	 * Processing: Insert/Merge
	 *
	 * rbase                                             rend
	 * +-------------------------------------------------+
	 * |                                                 |
	 * | Exist regions                                   |
	 * |                                                 |
	 * +-------------------------------------------------+
	 *
	 * 1) rbase: 0x60e00000
	 *    rend:  0x63000000
	 * 
	 */
	memblock_reserve(0x60e00000, 0x100000);
	pr_info("Scan forward and conjunct regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
			reg->base + reg->size);

	/*
	 * Insert a new region into memblock.reserved regions that disjunct 
	 * with exist regions, and the end address of new regions is more
	 * small than exist.
	 *
	 * memblock.reserved
	 *
	 * base                    end        rbase               rend
	 * +-----------------------+          +-------------------+
	 * |                       |          |                   | 
	 * | New region            |          | Exist regions     | 
	 * |                       |          |                   | 
	 * +-----------------------+          +-------------------+
	 *
	 * 1) end < rbase
	 * 2) rbase: 0x60e00000
	 *    rend:  0x63000000
	 *    base:  0x60a00000
	 *    end:   0x60b00000
	 */
	memblock_reserve(0x60a00000, 0x100000);
	pr_info("Scan forware and disjunct regions:\n");
	for_each_memblock(reserved, reg)
		pr_info("Region [%#x -- %#x]\n", reg->base,
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
