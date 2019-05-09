/*
 * cpuid
 *
 * (C) 2019.05.08 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/cputype.h>

static __init int cpuid_demo_init(void)
{
	unsigned int cpuid;

	cpuid = read_cpuid(CPUID_ID);

	printk("CPUID: %#lx\n", cpuid);	

	return 0;
}
device_initcall(cpuid_demo_init);
