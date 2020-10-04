/*
 * RDMSR [X86/i386/X86_64] -- Read from MSR register.
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
 * RDMSR -- Read from Model Specific Register
 *
 * Description
 * Read the contents of a 64-bit model specific register (MSR) specified in
 * the ECX register into register EDX:EAX. (On processors that support the
 * Intel 64 architecture, the high-order 32 bits of RCX are ignored.) The
 * EDX register is loaded with the high-order 32 bits of the MSR and the 
 * EAX register is loaded with the low-order 32 bits. (On processors that
 * support the Intel 64 architecture, the high-order 32 bits of each of RAX
 * and RDX are cleared.) If fewer than 64 bits are implemented in the MSR
 * being read, the values returned to EDX:EAX in unimplemented bit location
 * are undefined.
 *
 * This instruction must be executed at privilege level 0 or in real-address
 * mode; otherwise, a general protection execption #GP(0) will be generated.
 * Specifying a reserved or unimplemented MSR address in ECX will also cause
 * a general protection exception.
 *
 * The CPUID instruction should be used to determine whether MSRs are supported
 * (CPUID.01H:EDX[5] = 1) before using this instruction.
 */

/*
 * Both i386 and x86_64 returns 64-bit value in edx:eax, but gcc's "A"
 * constraint has different meanings. For i386, "A" means exactly
 * edx:eax, while for x86_64 it doesn't mean rdx:rax or edx:eax. Instead
 * it means rax *or* rdx.
 */
#ifdef CONFIG_X86_64
#define DECLARE_ARGS(val, low, high)		unsigned low, high
#define EAX_EDX_VAL(val, low, high)		((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)		"a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)		"=a" (low), "=d" (high)
#else
#define DECLARE_ARGS(val, low, high)		unsigned long long val
#define EAX_EDX_VAL(val, low, high)		(val)
#define EAX_EDX_ARGS(val, low, high)		"A" (val)
#define EAX_EDX_RET(val, low, high)		"=A" (val)
#endif

static inline unsigned long long native_read_msr_bs(unsigned int msr)
{
	DECLARE_ARGS(val, low, high);

	asm volatile("rdmsr"
		    : EAX_EDX_RET(val, low, high)
		    : "c" (msr));
	return EAX_EDX_VAL(val, low, high);
}

#define rdmsr_bs(msr, low, high)			\
do {							\
	u64 __val = native_read_msr_bs((msr));		\
	(void)((low) = (u32)__val);			\
	(void)((high) = (u32)(__val >> 32));		\
} while (0)

#define MSR_IA32_SYSENTER_CS		0x00000174

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	unsigned int host_cs, junk;

	/* RDMSR */
	rdmsr_bs(MSR_IA32_SYSENTER_CS, host_cs, junk);
	printk("MSR_IA32_SYSENTER_CS: %#lx-%lx\n", host_cs, junk);

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
