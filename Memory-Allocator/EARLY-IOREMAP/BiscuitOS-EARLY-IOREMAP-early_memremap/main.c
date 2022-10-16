/*
 * EARLY_IOREMAP Memory Alloctor: early_memremap
 *
 * Reserved memory: [64MiB, 64MiB + 4KiB)
 *   CMDLINE: memmap=4K$0x4000000
 *
 * (C) 2022.10.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-generic/early_ioremap.h>

#define RESERVED_MEMORY_BASE	0x4000000
#define RESERVED_MEMORY_SIZE	0x1000

int __init BiscuitOS_Running(void)
{
	void *mem;

	/* mapping */
	mem = early_memremap(RESERVED_MEMORY_BASE, RESERVED_MEMORY_SIZE);
	if (!mem) {
		printk("EARLY-IOREMAP failed\n");
		return -ENOMEM;
	}

	printk("Reserved Memory: %#x\n", *(unsigned int *)mem);

	/* unmapping */
	early_memunmap(mem, RESERVED_MEMORY_SIZE);

	return 0;
}
