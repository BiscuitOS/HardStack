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
#include <linux/kvm_host.h>
#include <linux/workqueue.h>
#include <asm/kvm_host.h>
#include <asm/fpu/internal.h>
#include <asm/intel_pt.h>
#include "kvm/internal.h"
#include "kvm/mmu.h"

#define KVM_NR_SHARED_MSRS	16

/* EFER defaults:
 * - enable syscall per default because its emulated by KVM
 * - enable LME and LMA per default on 64 bit KVM
 */
static u64 __read_mostly efer_reserved_bits_bs = ~((u64)EFER_SCE);

static unsigned int num_msr_based_features_bs;

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
	} values[KVM_NR_SHARED_MSRS];
};

struct kvm_x86_ops *kvm_x86_ops_bs __read_mostly;
EXPORT_SYMBOL_GPL(kvm_x86_ops_bs);
struct kmem_cache *x86_fpu_cache_bs;
EXPORT_SYMBOL_GPL(x86_fpu_cache_bs);
static unsigned long max_tsc_khz_bs;
static DEFINE_PER_CPU(unsigned long, cpu_tsc_khz_bs);
bool	__read_mostly kvm_has_tsc_control_bs;
EXPORT_SYMBOL_GPL(kvm_has_tsc_control_bs);
u64	__read_mostly kvm_max_tsc_scaling_ratio_bs;
EXPORT_SYMBOL_GPL(kvm_max_tsc_scaling_ratio_bs);
u8	__read_mostly kvm_tsc_scaling_ratio_frac_bits_bs;
EXPORT_SYMBOL_GPL(kvm_tsc_scaling_ratio_frac_bits_bs);

u64 __read_mostly host_xcr0_bs;
static struct kvm_shared_msrs_global __read_mostly shared_msrs_global_bs;
static struct kvm_shared_msrs __percpu *shared_msrs_bs;

static u32 msrs_to_save_bs[] = {
	MSR_IA32_SYSENTER_CS, MSR_IA32_SYSENTER_ESP, MSR_IA32_SYSENTER_EIP,
	MSR_STAR,
	MSR_IA32_TSC, MSR_IA32_CR_PAT, MSR_VM_HSAVE_PA,
	MSR_IA32_FEATURE_CONTROL, MSR_IA32_BNDCFGS, MSR_TSC_AUX,
	MSR_IA32_SPEC_CTRL, MSR_IA32_ARCH_CAPABILITIES,
	MSR_IA32_RTIT_CTL, MSR_IA32_RTIT_STATUS, MSR_IA32_RTIT_CR3_MATCH,
	MSR_IA32_RTIT_OUTPUT_BASE, MSR_IA32_RTIT_OUTPUT_MASK,
	MSR_IA32_RTIT_ADDR0_A, MSR_IA32_RTIT_ADDR0_B,
	MSR_IA32_RTIT_ADDR1_A, MSR_IA32_RTIT_ADDR1_B,
	MSR_IA32_RTIT_ADDR2_A, MSR_IA32_RTIT_ADDR2_B,
	MSR_IA32_RTIT_ADDR3_A, MSR_IA32_RTIT_ADDR3_B,
};

static unsigned num_msrs_to_save_bs;

/*
 * List of msr numbers which are used to expose MSR-based features that
 * can be used by a hypervisor to validate requested CPU features.
 */
static u32 msr_based_features_bs[] = {
	MSR_IA32_VMX_BASIC,
	MSR_IA32_VMX_TRUE_PINBASED_CTLS,
	MSR_IA32_VMX_PINBASED_CTLS,
	MSR_IA32_VMX_TRUE_PROCBASED_CTLS,
	MSR_IA32_VMX_PROCBASED_CTLS,
	MSR_IA32_VMX_TRUE_EXIT_CTLS,
	MSR_IA32_VMX_EXIT_CTLS,
	MSR_IA32_VMX_TRUE_ENTRY_CTLS,
	MSR_IA32_VMX_ENTRY_CTLS,
	MSR_IA32_VMX_MISC,
	MSR_IA32_VMX_CR0_FIXED0,
	MSR_IA32_VMX_CR0_FIXED1,
	MSR_IA32_VMX_CR4_FIXED0,
	MSR_IA32_VMX_CR4_FIXED1,
	MSR_IA32_VMX_VMCS_ENUM,
	MSR_IA32_VMX_PROCBASED_CTLS2,
	MSR_IA32_VMX_EPT_VPID_CAP,
	MSR_IA32_VMX_VMFUNC,

	MSR_F10H_DECFG,
	MSR_IA32_UCODE_REV,
	MSR_IA32_ARCH_CAPABILITIES,
};

static u32 emulated_msrs_bs[] = {
	MSR_KVM_SYSTEM_TIME, MSR_KVM_WALL_CLOCK,
	MSR_KVM_SYSTEM_TIME_NEW, MSR_KVM_WALL_CLOCK_NEW,
	HV_X64_MSR_GUEST_OS_ID, HV_X64_MSR_HYPERCALL,
	HV_X64_MSR_TIME_REF_COUNT, HV_X64_MSR_REFERENCE_TSC,
	HV_X64_MSR_TSC_FREQUENCY, HV_X64_MSR_APIC_FREQUENCY,
	HV_X64_MSR_CRASH_P0, HV_X64_MSR_CRASH_P1, HV_X64_MSR_CRASH_P2,
	HV_X64_MSR_CRASH_P3, HV_X64_MSR_CRASH_P4, HV_X64_MSR_CRASH_CTL,
	HV_X64_MSR_RESET,
	HV_X64_MSR_VP_INDEX,
	HV_X64_MSR_VP_RUNTIME,
	HV_X64_MSR_SCONTROL,
	HV_X64_MSR_STIMER0_CONFIG,
	HV_X64_MSR_VP_ASSIST_PAGE,
	HV_X64_MSR_REENLIGHTENMENT_CONTROL, HV_X64_MSR_TSC_EMULATION_CONTROL,
	HV_X64_MSR_TSC_EMULATION_STATUS,

	MSR_KVM_ASYNC_PF_EN, MSR_KVM_STEAL_TIME,
	MSR_KVM_PV_EOI_EN,

	MSR_IA32_TSC_ADJUST,
	MSR_IA32_TSCDEADLINE,
	MSR_IA32_MISC_ENABLE,
	MSR_IA32_MCG_STATUS,
	MSR_IA32_MCG_CTL,
	MSR_IA32_MCG_EXT_CTL,
	MSR_IA32_SMBASE,
	MSR_SMI_COUNT,
	MSR_PLATFORM_INFO,
	MSR_MISC_FEATURES_ENABLES,
	MSR_AMD64_VIRT_SPEC_CTRL,
};

static unsigned num_emulated_msrs_bs;

#define VCPU_STAT_BS(x)	offsetof(struct kvm_vcpu, stat.x), KVM_STAT_VCPU

__visible bool kvm_rebooting_bs;
EXPORT_SYMBOL_GPL(kvm_rebooting_bs);

asmlinkage __visible void kvm_spurious_fault_bs(void)
{
	/* Fault while not rebooting.  We want the trace. */
	BS_DUP();
	BUG();
}
EXPORT_SYMBOL_GPL(kvm_spurious_fault_bs);

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

struct kvm_stats_debugfs_item debugfs_entries_bs[] = {
	{ "pf_fixed", VCPU_STAT_BS(pf_fixed) },
	{ NULL }
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

u64 kvm_get_arch_capabilities_bs(void)
{
	u64 data;

	rdmsrl_safe(MSR_IA32_ARCH_CAPABILITIES, &data);

	/*
	 * If we're doing cache flushes (either "always" or "cond")
	 * we will do one whenever the guest does a vmlaunch/vmresume.
	 * If an outer hypervisor is doing the cache flush for us
	 * (VMENTER_L1D_FLUSH_NESTED_VM), we can safely pass that
	 * capability to the guest too, and if EPT is disabled we're not
	 * vulnerable.  Overall, only VMENTER_L1D_FLUSH_NEVER will
	 * require a nested hypervisor to do a flush of its own.
	 */
	if (l1tf_vmx_mitigation != VMENTER_L1D_FLUSH_NEVER)
		data |= ARCH_CAP_SKIP_VMENTRY_L1DFLUSH;

	return data;
}
EXPORT_SYMBOL_GPL(kvm_get_arch_capabilities_bs);

static int kvm_get_msr_feature_bs(struct kvm_msr_entry *msr)
{
	switch (msr->index) {
	case MSR_IA32_ARCH_CAPABILITIES:
		msr->data = kvm_get_arch_capabilities_bs();
		break;
	case MSR_IA32_UCODE_REV:
		rdmsrl_safe(msr->index, &msr->data);
		break;
	default:
		if (kvm_x86_ops_bs->get_msr_feature(msr))
			return 1;
	}
	return 0;
}

static void kvm_init_msr_list_bs(void)
{
	u32 dummy[2];
	unsigned i, j;

	for (i = j = 0; i < ARRAY_SIZE(msrs_to_save_bs); i++) {
		if (rdmsr_safe(msrs_to_save_bs[i], &dummy[0], &dummy[1]) < 0)
			continue;

		/*
		 * Even MSRs that are valid in the host may not be exposed
		 * to the guests in some cases.
		 */
		switch (msrs_to_save_bs[i]) {
		case MSR_IA32_BNDCFGS:
			if (!kvm_mpx_supported_bs())
				continue;
			break;
		case MSR_TSC_AUX:
			if (!kvm_x86_ops_bs->rdtscp_supported())
				continue;
			break;
		case MSR_IA32_RTIT_CTL:
		case MSR_IA32_RTIT_STATUS:
			BS_DUP();
			if (!kvm_x86_ops_bs->pt_supported())
				continue;
			break;
		case MSR_IA32_RTIT_CR3_MATCH:
			BS_DUP();
			if (!kvm_x86_ops_bs->pt_supported() ||
			    !intel_pt_validate_hw_cap(PT_CAP_cr3_filtering))
				continue;
			break;
		case MSR_IA32_RTIT_OUTPUT_BASE:
		case MSR_IA32_RTIT_OUTPUT_MASK:
			BS_DUP();
			if (!kvm_x86_ops_bs->pt_supported() ||
			   (!intel_pt_validate_hw_cap(PT_CAP_topa_output) &&
			    !intel_pt_validate_hw_cap(
						PT_CAP_single_range_output)))
				continue;
			break;
		case MSR_IA32_RTIT_ADDR0_A ... MSR_IA32_RTIT_ADDR3_B: {
			BS_DUP();
			if (!kvm_x86_ops_bs->pt_supported() ||
				msrs_to_save_bs[i] - MSR_IA32_RTIT_ADDR0_A >=
				intel_pt_validate_hw_cap(
						PT_CAP_num_address_ranges) * 2)
				continue;
			break;
		}
		default:
			break;
		}

		if (j < i)
			msrs_to_save_bs[j] = msrs_to_save_bs[i];
		j++;
	}
	num_msrs_to_save_bs = j;

	for (i = j = 0; i < ARRAY_SIZE(emulated_msrs_bs); i++) {
		if (!kvm_x86_ops_bs->has_emulated_msr(emulated_msrs_bs[i]))
			continue;

		if (j < i)
			emulated_msrs_bs[j] = emulated_msrs_bs[i];
		j++;
	}
	num_emulated_msrs_bs = j;

	for (i = j = 0; i < ARRAY_SIZE(msr_based_features_bs); i++) {
		struct kvm_msr_entry msr;

		msr.index = msr_based_features_bs[i];
		if (kvm_get_msr_feature_bs(&msr))
			continue;

		if (j < i)
			msr_based_features_bs[j] = msr_based_features_bs[i];
		j++;
	}
	num_msr_based_features_bs = j;
}

int kvm_arch_hardware_setup_bs(void)
{
	int r;

	r = kvm_x86_ops_bs->hardware_setup();
	if (r != 0)
		return r;

	if (kvm_has_tsc_control_bs) {
		BS_DUP();
	}

	kvm_init_msr_list_bs();
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

void kvm_enable_efer_bits_bs(u64 mask)
{
	efer_reserved_bits_bs &= ~mask;
}
EXPORT_SYMBOL_GPL(kvm_enable_efer_bits_bs);

void __kvm_request_immediate_exit_bs(struct kvm_vcpu *vcpu)
{
	smp_send_reschedule(vcpu->cpu);
}
EXPORT_SYMBOL_GPL(__kvm_request_immediate_exit_bs);

void kvm_arch_check_processor_compat_bs(void *rtn)
{
	kvm_x86_ops_bs->check_processor_compatibility(rtn);
}

static void kvmclock_update_fn_bs(struct work_struct *work)
{
	BS_DUP();
}

static void kvmclock_sync_fn_bs(struct work_struct *work)
{
	BS_DUP();
}

static void pvclock_update_vm_gtod_copy_bs(struct kvm *kvm)
{
}

int kvm_arch_init_vm_bs(struct kvm *kvm, unsigned long type)
{
	if (type)
		return -EINVAL;

	INIT_HLIST_HEAD(&kvm->arch.mask_notifier_list);
	INIT_LIST_HEAD(&kvm->arch.active_mmu_pages);
	INIT_LIST_HEAD(&kvm->arch.zapped_obsolete_pages);
	INIT_LIST_HEAD(&kvm->arch.assigned_dev_head);
	atomic_set(&kvm->arch.noncoherent_dma_count, 0);

	/* Reserve bit 0 of irq_sources_bitmap for userspace irq source */
	set_bit(KVM_USERSPACE_IRQ_SOURCE_ID, &kvm->arch.irq_sources_bitmap);
	/* Reserve bit 1 of irq_sources_bitmap for irqfd-resampler */
	set_bit(KVM_IRQFD_RESAMPLE_IRQ_SOURCE_ID,
				&kvm->arch.irq_sources_bitmap);

	raw_spin_lock_init(&kvm->arch.tsc_write_lock);
	mutex_init(&kvm->arch.apic_map_lock);
	spin_lock_init(&kvm->arch.pvclock_gtod_sync_lock);

	kvm->arch.kvmclock_offset = -ktime_get_boot_ns();
	pvclock_update_vm_gtod_copy_bs(kvm);

	kvm->arch.guest_can_read_msr_platform_info = true;

	INIT_DELAYED_WORK(&kvm->arch.kvmclock_update_work,
						kvmclock_update_fn_bs);
	INIT_DELAYED_WORK(&kvm->arch.kvmclock_sync_work,
						kvmclock_sync_fn_bs);

	kvm_hv_init_vm_bs(kvm);
	kvm_page_track_init_bs(kvm);
	kvm_mmu_init_vm_bs(kvm);

	if (kvm_x86_ops_bs->vm_init)
		return kvm_x86_ops_bs->vm_init(kvm);

	return 0;
}

static void shared_msr_update_bs(unsigned slot, u32 msr)
{
	u64 value;
	unsigned int cpu = smp_processor_id();
	struct kvm_shared_msrs *smsr = per_cpu_ptr(shared_msrs_bs, cpu);

	/* only read, and nobody should modify it at this time,
	 * so don't need lock */
	if (slot >= shared_msrs_global_bs.nr) {
		BS_DUP();
		return;
	}
	rdmsrl_safe(msr, &value);
	smsr->values[slot].host = value;
	smsr->values[slot].curr = value;
}

static void kvm_shared_msr_cpu_online_bs(void)
{
	unsigned i;

	for (i = 0; i < shared_msrs_global_bs.nr; ++i)
		shared_msr_update_bs(i, shared_msrs_global_bs.msrs[i]);
}

int kvm_arch_hardware_enable_bs(void)
{
	struct kvm *kvm;
	struct kvm_vcpu *vcpu;
	int i;
	int ret;
	u64 local_tsc;
	u64 max_tsc = 0;
	bool stable, backwards_tsc = false;

	kvm_shared_msr_cpu_online_bs();
	ret = kvm_x86_ops_bs->hardware_enable();
	if (ret != 0)
		return ret;

	return 0;
}
