/*
 * Paging Mechanism: Detect 32-bit paing mode
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
/* Header */
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/paravirt.h>
/* CPUID */
#include <asm/processor.h>
#include <asm/cpufeatures.h>

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned long cr0;
	unsigned long cr4;
	unsigned int eax, ebx, ecx, edx;

	/* Detect PG and PE on cr0 */
	cr0 = read_cr0();
	if (!(cr0 & X86_CR0_PG)) {
		printk("The System doesn't enable Paging mechanism.!\n");
		return 0;
	} else if ((cr0 & (X86_CR0_PG || X86_CR0_PE)) == 
					(X86_CR0_PG || X86_CR0_PE)) {
		printk("The System enable Protection mode and Paing.\n");
	} else {
		printk("Invald mode for PG and PE.\n");
		return -EINVAL;
	}

	/* Read CR4 and IA32_EFER MSR */
	cr4 = __read_cr4();

	/* Detect 32-bit paing */
	if (!(cr4 & X86_CR4_PAE))
		printk("The system uses 32-bit paging mode.\n");
	else {
		printk("Unknown paging mode.\n");
		return -EINVAL;
	}

	/* Paging With CPU FEATURE */
	/* CPUID.01H */
	eax = ebx = ecx = edx = 0;
	cpuid(0x01, &eax, &ebx, &ecx, &edx);

	printk("32-bit Paging FEATURE:\n");
	if (edx & X86_FEATURE_PSE36)
		printk("  CPUID.01H PSE36\n");
	if (edx & X86_FEATURE_PGE)
		printk("  CPUID.01H PGE\n");

	/* CPUID.80000008H */
	eax = ebx = ecx = edx = 0;
	cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
	printk("  MAXPHYADDR:   %d\n", eax & 0xff);
	printk("  Linear Width: %d\n", (eax >> 8) & 0xff);
	printk("  Virtual Range: %#lx\n", 
				(unsigned long)(1 << ((eax >> 8) & 0xff)));
	printk("  Physical Memory MAX: %#lx\n", 
				(unsigned long)(1 << (eax & 0xff)));

	/* Paging Capability */
	printk("32-bit Paging Capability:\n");
	if (cr0 & X86_CR0_WP)
		printk("  CR0.WP\n");
	if (cr0 & X86_CR4_PSE)
		printk("  CR4.PSE\n");
	if (cr0 & X86_CR4_PGE)
		printk("  CR4.PGE\n");

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
