/*
 * X86 KVM architecture
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/user-return-notifier.h>
#include <linux/cpufreq.h>
#include <asm/kvm_host.h>
#include <asm/fpu/internal.h>
#include "kvm/internal.h"
#include "kvm/mmu.h"

#define KVM_NR_SHARED_MSRS	16

struct kvm_shared_msrs_global {
	int nr;
	u32 msrs[KVM_NR_SHARED_MSRS];
};

struct kvm_shared_msrs {
	struct user_return_notifier urn;
	bool registered;
	struct kvm_shared_msr_values {
		u64 host;
		u64 curr;
	} value[KVM_NR_SHARED_MSRS];
};

struct kvm_x86_ops *kvm_x86_ops_bs __read_mostly;
EXPORT_SYMBOL_GPL(kvm_x86_ops_bs);
struct kmem_cache *x86_fpu_cache_bs;
EXPORT_SYMBOL_GPL(x86_fpu_cache_bs);
static unsigned long max_tsc_khz_bs;
static DEFINE_PER_CPU(unsigned long, cpu_tsc_khz_bs);

u64 __read_mostly host_xcr0_bs;
static struct kvm_shared_msrs_global __read_mostly shared_msrs_global_bs;
static struct kvm_shared_msrs __percpu *shared_msrs_bs;

static void tsc_khz_changed_bs(void *data)
{
	struct cpufreq_freqs *freq = data;
	unsigned long khz = 0;

	if (data)
		khz = freq->new;
	else if (!boot_cpu_has(X86_FEATURE_CONSTANT_TSC))
		khz = cpufreq_quick_get(raw_smp_processor_id());
	if (!khz)
		khz = tsc_khz;
	__this_cpu_write(cpu_tsc_khz_bs, khz);
}


static int kvmclock_cpufreq_notifier_bs(struct notifier_block *nb,
						unsigned long val, void *data)
{
	BS_DUP();
	return 0;
}

static int kvmclock_cpu_online_bs(unsigned int cpu)
{
	tsc_khz_changed_bs(NULL);
	return 0;
}

static int kvmclock_cpu_down_prep_bs(unsigned int cpu)
{
	BS_DUP();
	return 0;
}

static struct notifier_block kvmclock_cpufreq_notifier_block_bs = {
	.notifier_call = kvmclock_cpufreq_notifier_bs,
};

static void kvm_set_mmio_spte_mask_bs(void)
{
	u64 mask;
	int maxphyaddr = boot_cpu_data.x86_phys_bits;

	/*
	 * Set the reserved bits and the present bit of an paging-structure
	 * entry to generate page fault with PFER.RSV = 1.
	 */

	/*
	 * Mask the uppermost physical address bit, which would be reserved
	 * as long as the supported physical address width is less than 52.
	 */
	mask = 1ull << 51;

	/* Set the present bit. */
	mask |= 1ull;

	/*
	 * If reserved bit is not supported, clear the present bit to disable
	 * mmio page fault.
	 */
	if (IS_ENABLED(CONFIG_X86_64) && maxphyaddr == 52)
		mask &= ~1ull;

	kvm_mmu_set_mmio_spte_mask_bs(mask, mask);
}

static void kvm_timer_init_bs(void)
{
	max_tsc_khz_bs = tsc_khz;

	if (!boot_cpu_has(X86_FEATURE_CONSTANT_TSC)) {
		cpufreq_register_notifier(&kvmclock_cpufreq_notifier_block_bs,
				CPUFREQ_TRANSITION_NOTIFIER);
	}
	printk("kvm: max_tsc_khz = %ld\n", max_tsc_khz_bs);

	cpuhp_setup_state(CPUHP_AP_X86_KVM_CLK_ONLINE, "x86/kvm/clk:online",
			kvmclock_cpu_online_bs, kvmclock_cpu_down_prep_bs);
}

int kvm_is_in_guest_bs(void)
{
	BS_DUP();
	return 0;
}

static int kvm_is_user_mode_bs(void)
{
	BS_DUP();
	return 0;
}

static unsigned long kvm_get_guest_ip_bs(void)
{
	BS_DUP();
	return 0;
}

static struct perf_guest_info_callbacks kvm_guest_cbs_bs = {
	.is_in_guest	= kvm_is_in_guest_bs,
	.is_user_mode	= kvm_is_user_mode_bs,
	.get_guest_ip	= kvm_get_guest_ip_bs,
};

int kvm_arch_init_bs(void *opaque)
{
	int r;
	struct kvm_x86_ops *ops = opaque;

	if (kvm_x86_ops_bs) {
		printk("kvm: already loaded the other module\n");
		r = -EEXIST;
		goto out;
	}

	if (!ops->cpu_has_kvm_support()) {
		printk("kvm: no hardware support\n");
		r = -EOPNOTSUPP;
		goto out;
	}
	if (ops->disabled_by_bios()) {
		printk("kvm: disabled by bios\n");
		r = -EOPNOTSUPP;
		goto out;
	}

	/*
	 * KVM explicitly assumes that the guest has an FPU and
	 * FXSAVE/FXRSTOR. For example, the KVM_GET_FPU explicitly casts the
	 * vCPU's FPU state as a fxregs_state struct.
	 */
	if (!boot_cpu_has(X86_FEATURE_FPU) || !boot_cpu_has(X86_FEATURE_FXSR)) {
		printk("kvm: inadequate fpu\n");
		r = -EOPNOTSUPP;
		goto out;
	}

	r = -ENOMEM;
	x86_fpu_cache_bs = kmem_cache_create("x86_fpu_bs", 
					sizeof(struct fpu),
					__alignof__(struct fpu), SLAB_ACCOUNT,
					NULL);
	if (!x86_fpu_cache_bs) {
		printk("kvm: failed to allocate cache for x86 fpu\n");
		goto out;
	}

	shared_msrs_bs = alloc_percpu(struct kvm_shared_msrs);
	if (!shared_msrs_bs) {
		printk("kvm: failed to allocate percpu kvm_shared_msrs\n");
		goto out_free_x86_fpu_cache;
	}

	r = kvm_mmu_module_init_bs();
	if (r)
		goto out_free_percpu;

	kvm_set_mmio_spte_mask_bs();

	kvm_x86_ops_bs = ops;

	kvm_mmu_set_mask_ptes_bs(PT_USER_MASK, PT_ACCESSED_MASK,
			PT_DIRTY_MASK, PT64_NX_MASK, 0,
			PT_PRESENT_MASK, 0, sme_me_mask);

	kvm_timer_init_bs();

	perf_register_guest_info_callbacks(&kvm_guest_cbs_bs);

	if (boot_cpu_has(X86_FEATURE_XSAVE))
		host_xcr0_bs = xgetbv(XCR_XFEATURE_ENABLED_MASK);

	kvm_lapic_init_bs();

	return 0;
out_free_percpu:
	free_percpu(shared_msrs_bs);
out_free_x86_fpu_cache:
	kmem_cache_destroy(x86_fpu_cache_bs);
out:
	return r;
}

int kvm_arch_hardware_setup_bs(void)
{
	int r;

	r = kvm_x86_ops_bs->hardware_setup();
	if (r != 0)
		return r;
	return 0;
}

void kvm_define_shared_msr_bs(unsigned slot, u32 msr)
{
	BUG_ON(slot >= KVM_NR_SHARED_MSRS);
	shared_msrs_global_bs.msrs[slot] = msr;
	if (slot >= shared_msrs_global_bs.nr)
		shared_msrs_global_bs.nr = slot + 1;
}
EXPORT_SYMBOL_GPL(kvm_define_shared_msr_bs);
