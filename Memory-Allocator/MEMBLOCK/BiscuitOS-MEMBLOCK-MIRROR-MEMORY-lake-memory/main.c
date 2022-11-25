/*
 * MEMBLOCK Memory Allocator: Memory Mirror Lake
 *
 * - must support 'kernelcore=mirror' on CMDLINE
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

/* BiscuitOS NUMA Layout
 *  - Layout0: 2CPUs with 2 NUMA NODE
 *    NODE0: 0x00000000 - 0x100000000
 *    NODE1: 0x10000000 - 0x200000000
 *
 *  - Layout1: 4CPUs with 3 NUMA NODE
 *    NODE0: 0x00000000 - 0x200000000
 *    NODE1: 0x20000000 - 0x300000000
 *    NODE2: 0x30000000 - 0x400000000
 */
#define MEMBLOCK_MIRROR_BASE	0x22000000
#define MEMBLOCK_MIRROR_SIZE	0x1000
#define MEMBLOCK_MIRROR_END	(MEMBLOCK_MIRROR_BASE + MEMBLOCK_MIRROR_SIZE)
#define MEMBLOCK_FAKE_SIZE	0x2000
#define MEMBLOCK_FAKE_BASE	(MEMBLOCK_MIRROR_BASE)
#define MEMBLOCK_FAKE_END	(MEMBLOCK_MIRROR_BASE + 3 * MEMBLOCK_MIRROR_SIZE)
#define MEMBLOCK_FAKE_NODE	1

int __init BiscuitOS_Running(void)
{
	phys_addr_t phys, start, end;
	void *mem;
	int nid;
	u64 idx;

	/* Iterate free memory */
	printk("==== MEMBLOCK NODE %d Free Area ====\n", MEMBLOCK_FAKE_NODE);
	for_each_free_mem_range(idx, MEMBLOCK_FAKE_NODE, MEMBLOCK_NONE,
							&start, &end, &nid)
		printk("FreeRange: %#llx - %#llx - NID %d\n",
							start, end, nid);

	/* Mark Memory Mirrorable */
	memblock_mark_mirror(MEMBLOCK_MIRROR_BASE, MEMBLOCK_MIRROR_SIZE);

	/* Alloc Memory */
	phys = memblock_alloc_range_nid(MEMBLOCK_FAKE_SIZE,
				SMP_CACHE_BYTES, MEMBLOCK_FAKE_BASE,
				MEMBLOCK_FAKE_END, MEMBLOCK_FAKE_NODE, true);
	if (!phys) {
		printk("FATAL ERROR: No Free Memory on MEMBLOCK.\n");
		return -ENOMEM;
	}

	mem = phys_to_virt(phys);
	sprintf(mem, "Hello %s", "BiscuitOS");
	printk("==== %s ====\n", (char *)mem);
	printk("Physical Address %#lx\n", __pa(mem));

	/* Free Memory */
	memblock_free(mem, MEMBLOCK_FAKE_SIZE);

	return 0;
}
