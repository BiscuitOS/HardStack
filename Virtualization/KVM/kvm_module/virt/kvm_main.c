/*
 * kvm_main.c
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/slab.h>
#include "kvm/internal.h"

static cpumask_var_t cpus_hardware_enabled_bs;

int kvm_init_bs(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
				struct module *module)
{
	int r;

	r = kvm_arch_init_bs(opaque);
	if (r)
		goto out_fail;

	/*
	 * kvm_arch_init makes sure there's at most one caller
	 * for architectures that support multiple implementations,
	 * like intel and amd on x86.
	 * kvm_arch_init must be called before kvm_irqfd_init to avoid
	 * creating conflicts on case kvm is already setup for another
	 * implementation.
	 */
	r = kvm_irqfd_init_bs();
	if (r)
		goto out_irqfd;

	if (!zalloc_cpumask_var(&cpus_hardware_enabled_bs, GFP_KERNEL)) {
		r = -ENOMEM;
		goto out_free_0;
	}

	r = kvm_arch_hardware_setup_bs();
	if (r < 0)
		goto out_free_0a;

	return 0;
out_free_0a:
	//free_cpumask_var(cpu_hardware_enabled_bs);
out_free_0:
	//kvm_irqfd_exit();
out_irqfd:
	//kvm_arch_exit();
out_fail:
	return r;
}
EXPORT_SYMBOL_GPL(kvm_init_bs);
