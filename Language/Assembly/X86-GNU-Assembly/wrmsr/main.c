/*
 * WRMSR [X86/X86_64] -- Write to Model Specific Register
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

/*
 * WRMSR -- Write to Model Specific Register
 *
 * Description
 * Writes the contents of registers EDX:EAX into the 64-bit model specific
 * register (MSR) specified in the ECX register. (On processors that support
 * the Intel 64 architecture, the high-order 32 bits of RCX are ignored.)
 * The contents of the EDX register are copied to high-order 32 bits of the
 * selected MSR and the contents of the EAX register are copied to low-order
 * 32 bits of the MSR. (On processors that support the Intel 64 architecture,
 * the high-order 32 bits of each of RAX and RDX are ignore.) Undefined or
 * reserved bits in an MSR should be set to values previously read.
 *
 * This instrution must be executed at privilege level 0 or in real-address
 * mode; otherwise, a general protection exception #GP(0) is generated. 
 * Specifying a reserved or unimplemented MSR address in ECX will also cause
 * a general protection exception. The processor will also generate a general
 * protection exception if software attempts to write to bits in a reserved
 * MSR.
 */

#define IA32_PQR_ASSOC		0x0c8f

static inline void wrmsr_bs(unsigned msr, unsigned low, unsigned high)
{
	asm volatile ("wrmsr"
		      :
		      : "c" (msr), "a" (low), "d" (high)
		      : "memory");
}

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	printk("Hello modules on BiscuitOS\n");

	/* WRMSR */
	wrmsr_bs(IA32_PQR_ASSOC, 0, 0);

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
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
