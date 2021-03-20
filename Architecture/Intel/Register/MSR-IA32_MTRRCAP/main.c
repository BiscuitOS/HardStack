/*
 * Intel IA32_MTRRCAP MSR
 *
 * 64                                                        9 8              0
 * +----------------------------------------------------+-+-+-+-+--------------+
 * |                                                    | | | |F|              |
 * |                      Reserved                      | |W| |I|     VCNT     |
 * |                                                    | |C| |X|              |
 * +----------------------------------------------------+-+-+-+-+--------------+
 *                                                       | |   |        |
 *                                                       | |   |        |
 *                                                       | |   |        |
 * SMRR ---- SMRR interface support ---------------------o |   |        |
 * WC   ---- Write-combining memory type support ----------o   |        |
 * FIX  ---- Fixed range registers support --------------------o        |
 * VCNT ---- Number of variable range register -------------------------o
 *
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned int lo, hi;
	unsigned int eax, ebx, ecx, edx;

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

	/* Read IA32_MTRRCAP MSR */
	rdmsr(MSR_MTRRcap, lo, hi);
	printk("MSR_MTRRcap:\n");

	/* VCNT */
	printk("  The number of Variable-Range Register: %d\n", lo & 0xff);

	/* FIX */
	printk("  FIX:   %s\n", lo & 0x100 ? "Support" : "Drop");

	/* WC */
	printk("  WC:    %s\n", lo & 0x400 ? "Support" : "Drop");

	/* SMRR */
	printk("  SMRR:  %s\n", lo & 0x800 ? "Support" : "Drop");

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
