/*
 * MEMBLOCK Allocator: NO-MAP Region
 *
 * - Run on ARM Architecture
 * - Keep Enable CONFIG_ARCH_KEEP_MEMBLOCK
 * - DTS on vexpress-v2p-ca9.dts
 *   
 *   reserved-memory {
 *        ...
 *        BiscuitOS: BiscuitOS@70000000 {
 *            compatible = "BiscuitOS-Reserved";
 *            reg = <0x70000000 0x00000000>;
 *            size = <0x02000000>;
 *            alloc-ranges = <0x70000000 0x02000000>;
 *            no-map;
 *        };
 *   };
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <asm/io.h>

/* kernel entry on initcall */
static int __init BiscuitOS_init(void)
{
	struct memblock_region *region;
	phys_addr_t addr = 0;

	/* Iterate nomap regions */
	printk("==== Iterate NO-MAP Region ====\n");
	for_each_mem_region(region)
		if (memblock_is_nomap(region)) {
			printk("Region: %#x - %#x\n", (u32)region->base,
					(u32)(region->base + region->size));
			addr = region->base;
		}

	if (addr) {
		char *mem = ioremap(addr, 0x1000);

		sprintf(mem, "Hello %s", "BiscuitOS");
		printk("=== %s ===\n", mem);

		iounmap(mem);
	}

	return 0;
}
device_initcall(BiscuitOS_init);
