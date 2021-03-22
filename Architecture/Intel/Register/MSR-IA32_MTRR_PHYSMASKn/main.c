/*
 *
 * IA32_MTRR_PHYSMASKn Register
 *
 * 64                         MAXPHYADDR           12   10                    0
 * +--------------------------+----------------------+-+-----------------------+
 * |                          |                      | |                       |
 * |         Reserved         |       PhysMask       |V|        Reserved       |
 * |                          |                      | |                       |
 * +--------------------------+----------------------+-+-----------------------+
 *                                       |            |
 *                                       |            |
 *                                       |            |
 * PhysMask -- Sets range mask ----------o            |
 * V        -- Valid ---------------------------------o
 *
 *
 * (C) 2021.03.21 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* MSR Header */
#include <asm/msr.h>
#include <asm/msr-index.h>
/* CPUID Header */
#include <asm/processor.h>
#include <asm/cpufeatures.h>
/* MTRR */
#include <asm/mtrr.h>

#define IA32_MTRR_BASEPHYS2	0x204
#define IA32_MTRR_BASEMASK2	0x205
#define PHYS_SIZE		0x100000

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned int eax, ebx, ecx, edx;
	phys_addr_t base;
	u64 val, mask;
	int maxphys;
	int type;

	eax = ebx = ecx = edx = 0;
	/* CPUID.01H */
	cpuid(0x01, &eax, &ebx, &ecx, &edx);

	/* MTRR */
	if (edx & (1 << 12))
		printk("CPUID.01 EDX Support MTRR\n");
	else {
		printk("Architecture doesn't support MTRR.\n");
		return 0;
	}

	eax = ebx = ecx = edx = 0;
	/* CPUID.80000008H */
	cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
	maxphys = eax & 0xFF;

	printk("Physical Range: 0x200000 - 0x2FFFFF\n");
	/* IA32_MTRR_BASEPHYS2 */
	base = 0x200000;
	mask = (1 << (maxphys - PAGE_SHIFT)) - 1;
	type = 0x06; /* write-back */
	val = (((base >> PAGE_SHIFT) & mask) << PAGE_SHIFT) | type;
	printk("IA32_MTRR_BASEPHYS2: %#llx\n", val);
	wrmsrl(IA32_MTRR_BASEPHYS2, val);

	/* IA32_MTRR_BASEMASK2 */
	mask = ((base ^ (~(PHYS_SIZE - 1))) >> PAGE_SHIFT) &
				((1 << (maxphys - PAGE_SHIFT)) - 1);
	mask <<= PAGE_SHIFT;
	mask |= 1 << 11;
	printk("IA32_MTRR_BASEMASK2: %#llx\n", mask);
	wrmsrl(IA32_MTRR_BASEMASK2, mask);

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Intel Register on BiscuitOS");
