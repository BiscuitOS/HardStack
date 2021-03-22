/*
 * MSR IA32_MTRR_FIX4K_E8000
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

#define S4K	0x1000
#define BASE	0xE8000

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
	unsigned int eax, ebx, ecx, edx;
	u64 val;
	int i;

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

	/* Read IA32_MTRR_FIX4K_E8000 MSR */
	rdmsrl(MSR_MTRRfix4K_E8000, val);
	printk("MSR IA32_MTRR_FIX4K_E8000:\n");

	for (i = 0; i < 8; i++)
		printk("  [%#08x - %#08x]: %s\n", BASE + i * S4K, 
				BASE + (i + 1) * S4K, 
				mtrr_attrib_to_str((val >> (i * 8)) & 0xFF));

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
