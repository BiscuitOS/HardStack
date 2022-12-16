/*
 * MTRRs: Detect
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
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <uapi/asm/mtrr.h>

static int __init BiscuitOS_init(void)
{
	unsigned long val, dummy;
	unsigned int num_var_msr;
	int i;

	if (!(cpuid_edx(0x80000001) & X86_FEATURE_MTRR)) {
		printk("System Doesn't MTRR MSR.\n");
		return 0;
	} else
		printk("System Support MTRR MSR.\n");

	/* IA32_MTRRCAP Register */
	rdmsr(MSR_MTRRcap, val, dummy);
	num_var_msr = val & 0xff;
	printk("Number of variable range register: %d\n", num_var_msr);
	if (val & 0x100)
		printk("Support Fixed range registers.\n");
	if (val & 0x400)
		printk("Support Write-combining memory type.\n");
	if (val & 0x800)
		printk("Support SMRR interface.\n");

	/* Default Type */
	rdmsr(MSR_MTRRdefType, val, dummy);
	printk("MTRR Default Type: %#lx\n", val & 0xff);
	if (val & 0x800) {
		printk("Enable all MTRRs.\n");
		if (val & 0x400)
			printk("Enable Fixed-range/Variable-range MTRRs.\n");
		else
			printk("Only Enable Variable-range MTRRs.\n");
	} else
		printk("Disable all MTRRs.\n");

	/* Vaiable-Range */
	for (i = 0; i < num_var_msr; i++) {
		unsigned int hi, lo;

		rdmsr(MTRRphysBase_MSR(i), lo, hi);
		printk("VR %d Base:  %#x%08x\n", i, hi, lo);
		rdmsr(MTRRphysMask_MSR(i), lo, hi);
		printk("     Mask:  %#x%08x\n", hi, lo);
	}

	/* Fixed-Range */
	rdmsr(MSR_MTRRfix64K_00000, val, dummy);
	printk("FIX64K_00000: %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix16K_80000, val, dummy);
	printk("FIX16K_80000: %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix16K_A0000, val, dummy);
	printk("FIX16K_A0000: %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_C0000, val, dummy);
	printk("FIX4K_C0000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_C8000, val, dummy);
	printk("FIX4K_C8000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_D0000, val, dummy);
	printk("FIX4K_D0000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_D8000, val, dummy);
	printk("FIX4K_D8000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_E0000, val, dummy);
	printk("FIX4K_E0000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_E8000, val, dummy);
	printk("FIX4K_E8000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_F0000, val, dummy);
	printk("FIX4K_F0000:  %#lx%08lx\n", dummy, val);
	rdmsr(MSR_MTRRfix4K_F8000, val, dummy);
	printk("FIX4K_F8000:  %#lx%08lx\n", dummy, val);

	return 0;
}

static void __exit BiscuitOS_exit(void) { }

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("X86 MTRRs on BiscuitOS");
