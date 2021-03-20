/*
 *
 * IA32_MTRR_DEF_TYPE MSR
 *
 * 64                                                        9 8              0
 * +----------------------------------------------------+-+-+---+--------------+
 * |                                                    | | |   |              |
 * |                      Reserved                      |E|F|   |     Type     |
 * |                                                    | |E|   |              |
 * +----------------------------------------------------+-+-+---+--------------+
 *                                                       | |            |
 *                                                       | |            |
 *                                                       | |            |
 * E    ---- MTRR enable/disable ------------------------o |            |
 * FE   ---- Fixed-range MTRRs enable/disable -------------o            |
 * Type ---- Default memory type ---------------------------------------o
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
/* MTRR */
#include <asm/mtrr.h>

static const char *const mtrr_strings[MTRR_NUM_TYPES] =
{
	"uncachable",           /* 0 */
	"write-combining",      /* 1 */
	"?",                    /* 2 */
	"?",                    /* 3 */
	"write-through",        /* 4 */
	"write-protect",        /* 5 */
	"write-back",           /* 6 */
};

const char *mtrr_attrib_to_str(int x)
{
	return (x <= 6) ? mtrr_strings[x] : "?";
}

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

	/* Read IA32_MTRR_DEF_TYPE MSR */
	rdmsr(MSR_MTRRdefType, lo, hi);
	printk("MSR_MTRR_DEF_TYPE:\n");

	/* MTRR Enable */
	if (lo & (1 << 11)) {
		printk("  MTRR Enable.\n");

		/* FE */
		printk("  Fixed-Range Register: %s\n", lo & (1 << 10) ? 
							"Enable" : "Disable");

		/* Default Type */
		printk("  Default Type: %s\n", mtrr_attrib_to_str(lo & 0xFF));
	} else
		printk("  MTRR Doesn't enable, and default memory type: UC\n");

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
