/*
 * UP-PERCPU Memory Allocator
 *
 * (C) 2020.02.19 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "linux/biscuitos.h"
#include "linux/memblock.h"
#include "linux/percpu.h"

struct bs_struct {
	char name[32];
};

/* percpu alloc */
static int instance_percpu_alloc(void)
{
	struct bs_struct __percpu *bs;
	struct bs_struct *bp;
	unsigned int cpu;

	/* alloc */
	bs = alloc_percpu(struct bs_struct);

	/* setup all */
	for_each_possible_cpu(cpu) {
		bp = per_cpu_ptr(bs, cpu);
		sprintf(bp->name, "BiscuitOS-%d", cpu);
	}

	bp = per_cpu_ptr(bs, 1);
	printk("CPU %s\n", bp->name);

	/* free */
	free_percpu(bs);

	return 0;
}

int main()
{
	memory_init();

	/* Running instance */
	instance_percpu_alloc();

	memory_exit();

	return 0;
}
