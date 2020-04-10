/*
 * X86 KVM cpuid
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/fpu/types.h>

#include "kvm/internal.h"

bool kvm_mpx_supported_bs(void)
{
	BS_DUP();
	return ((host_xcr0_bs & (XFEATURE_MASK_BNDREGS | XFEATURE_MASK_BNDCSR))
		&& kvm_x86_ops_bs->mpx_supported());
}
EXPORT_SYMBOL_GPL(kvm_mpx_supported_bs);
