/*
 * KVM: vmx
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/tboot.h>
#include <linux/kvm_host.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/smp.h>
#include <linux/sched/smt.h>
#include <asm/mshyperv.h>
#include <asm/mce.h>
#include <asm/processor.h>
#include <asm/vmx.h>
#include <asm/kexec.h>
#include <asm/tlbflush.h>

#include "kvm/internal.h"
#include "kvm/vmx.h"
#include "kvm/virtext.h"
#include "kvm/capabilities.h"
#include "kvm/x86.h"
#include "kvm/evmcs.h"
#include "kvm/ops.h"
#include "kvm/vmcs12.h"

#define KVM_VMX_TSC_MULTIPLIER_MAX	0xffffffffffffffffULL

/* Guest_tsc -> host_tsc conversion requires 64-bit division. */
static int __read_mostly cpu_preemption_timer_multi_bs;
static bool __read_mostly enable_preemption_timer_bs = 1;

u64 host_efer_bs;

static u64 __read_mostly host_xss_bs;

bool __read_mostly enable_vpid_bs = 1;
module_param_named(vpid, enable_vpid_bs, bool, 0444);

bool __read_mostly enable_ept_bs = 1;
module_param_named(ept, enable_ept_bs, bool, S_IRUGO);

bool __read_mostly enable_ept_ad_bits_bs = 1;
module_param_named(eptad, enable_ept_ad_bits_bs, bool, S_IRUGO);

bool __read_mostly enable_unrestricted_guest_bs = 1;
module_param_named(unrestricted_guest,
			enable_unrestricted_guest_bs, bool, S_IRUGO);

bool __read_mostly flexpriority_enabled_bs = 1;
module_param_named(flexpriority, flexpriority_enabled_bs, bool, S_IRUGO);

static bool __read_mostly enable_vnmi_bs = 1;
module_param_named(vnmi, enable_vnmi_bs, bool, S_IRUGO);

static bool __read_mostly enable_apicv_bs = 1;
module_param(enable_apicv_bs, bool, S_IRUGO);

bool __read_mostly enable_pml_bs = 1;
module_param_named(pmd, enable_pml_bs, bool, S_IRUGO);
        
static bool __read_mostly emulate_invalid_guest_state_bs = true;
module_param(emulate_invalid_guest_state_bs, bool, S_IRUGO);

/*
 * These 2 parameters are used to config the controls for Pause-Loop Exiting:
 * ple_gap:    upper bound on the amount of time between two successive
 *             executions of PAUSE in a loop. Also indicate if ple enabled.
 *             According to test, this time is usually smaller than 128 cycles.
 * ple_window: upper bound on the amount of time a guest is allowed to execute
 *             in a PAUSE loop. Tests indicate that most spinlocks are held for
 *             less than 2^12 cycles
 * Time is measured based on a counter that runs at the same rate as the TSC,
 * refer SDM volume 3b section 21.6.13 & 22.1.3.
 */
static unsigned int ple_gap_bs = KVM_DEFAULT_PLE_GAP;
module_param(ple_gap_bs, uint, 0444);

static unsigned int ple_window_bs = KVM_VMX_DEFAULT_PLE_WINDOW;
module_param(ple_window_bs, uint, 0444);

/* Default doubles per-vcpu window every exit. */
static unsigned int ple_window_grow_bs = KVM_DEFAULT_PLE_WINDOW_GROW;
module_param(ple_window_grow_bs, uint, 0444);

/* Default resets per-vcpu window every exit to ple_window. */
static unsigned int ple_window_shrink_bs = KVM_DEFAULT_PLE_WINDOW_SHRINK;
module_param(ple_window_shrink_bs, uint, 0444);

/* Default is to compute the maximum so we can never overflow. */
static unsigned int ple_window_max_bs = KVM_VMX_DEFAULT_PLE_WINDOW_MAX;
module_param(ple_window_max_bs, uint, 0444);

/* Default is SYSTEM mode, 1 for host-guest mode */
int __read_mostly pt_mode_bs = PT_MODE_SYSTEM;
module_param(pt_mode_bs, int, S_IRUGO);

/*
 * If nested=1, nested virtualization is supported, i.e., guests may use
 * VMX and be a hypervisor for its own guests. If nested=0, guests may not
 * use VMX instructions.
 */
static bool __read_mostly nested_bs = 1;
module_param(nested_bs, bool, S_IRUGO);

/*
 * Through SYSCALL is only supported in 64-bit mode on Intel CPUs, kvm
 * will emulate YSCALL in legacy mode if the vendor string in guest
 * CPUID.0:{EBX,ECX,EDX} is "AuthenticAMD" or "AMDisbetter!" To
 * support this emulation, IA32_STAR must always be included in
 * vmx_msr_index[], even in i386 builds.
 */
const u32 vmx_msr_index_bs[] = {
	MSR_EFER, MSR_TSC_AUX, MSR_STAR,
};

struct vmcs_config vmcs_config_bs;
struct vmx_capability vmx_capability_bs;

static DECLARE_BITMAP(vmx_vpid_bitmap_bs, VMX_NR_VPIDS);

u64 __read_mostly kvm_mce_cap_supported_bs = MCG_CTL_P | MCG_SER_P;
EXPORT_SYMBOL_GPL(kvm_mce_cap_supported_bs);

static DEFINE_PER_CPU(struct vmcs *, vmxarea_bs);
DEFINE_PER_CPU(struct vmcs *, current_vmcs_bs);

/* Storage for pre module init parameter parsing */
static enum vmx_l1d_flush_state __read_mostly vmentry_l1d_flush_param_bs = 
				VMENTER_L1D_FLUSH_AUTO;

static DEFINE_STATIC_KEY_FALSE(vmx_l1d_should_flush_bs);
static DEFINE_STATIC_KEY_FALSE(vmx_l1d_flush_cond_bs);
static DEFINE_MUTEX(vmx_l1d_flush_mutex_bs);

#define L1D_CACHE_ORDER	4

static void *vmx_l1d_flush_pages_bs;

/*
 * We maintain a per-CPU linked-list of vCPU, so in wakeup_handler() we
 * can find which vCPU should be waken up.
 */
static DEFINE_PER_CPU(struct list_head, blocked_vcpu_on_cpu_bs);
static DEFINE_PER_CPU(spinlock_t, blocked_vcpu_on_cpu_lock_bs);

/*
 * We maintain a per-CPU linked-list of VMCS loaded on that CPU. This is needed
 * when a CPU is brought down, and we need to VMCLEAR all VMCSs loaded on it.
 */
static DEFINE_PER_CPU(struct list_head, loaded_vmcss_on_cpu_bs);

/*
 * Comment's format: document - errata name - stepping - processor name.
 * Refer from
 * https://www.virtualbox.org/svn/vbox/trunk/src/VBox/VMM/VMMR0/HMR0.cpp
 */
static u32 vmx_preemption_cpu_tfms_bs[] = {
/* 323344.pdf - BA86   - D0 - Xeon 7500 Series */
0x000206E6,
/* 323056.pdf - AAX65  - C2 - Xeon L3406 */
/* 322814.pdf - AAT59  - C2 - i7-600, i5-500, i5-400 and i3-300 Mobile */
/* 322911.pdf - AAU65  - C2 - i5-600, i3-500 Desktop and Pentium G6950 */
0x00020652,
/* 322911.pdf - AAU65  - K0 - i5-600, i3-500 Desktop and Pentium G6950 */
0x00020655,
/* 322373.pdf - AAO95  - B1 - Xeon 3400 Series */
/* 322166.pdf - AAN92  - B1 - i7-800 and i5-700 Desktop */
/*
 * 320767.pdf - AAP86  - B1 -
 * i7-900 Mobile Extreme, i7-800 and i7-700 Mobile
 */
0x000106E5,
/* 321333.pdf - AAM126 - C0 - Xeon 3500 */
0x000106A0,
/* 321333.pdf - AAM126 - C1 - Xeon 3500 */
0x000106A1,
/* 320836.pdf - AAJ124 - C0 - i7-900 Desktop Extreme and i7-900 Desktop */
0x000106A4,
 /* 321333.pdf - AAM126 - D0 - Xeon 3500 */
 /* 321324.pdf - AAK139 - D0 - Xeon 5500 */
 /* 320836.pdf - AAJ124 - D0 - i7-900 Extreme and i7-900 Desktop */
0x000106A5,
 /* Xeon E3-1220 V2 */
0x000306A8,
};

static inline bool cpu_has_broken_vmx_preemption_timer_bs(void)
{
	u32 eax = cpuid_eax(0x00000001), i;

	/* Clear the reserved bits */
	eax &= ~(0x3U << 14 | 0xfU << 28);
	for (i = 0; i < ARRAY_SIZE(vmx_preemption_cpu_tfms_bs); i++)
		if (eax == vmx_preemption_cpu_tfms_bs[i])
			return true;
	return false;
}

#define L1TF_MSG_SMT_BS "L1TF CPU bug present and SMT on, data leak possible. See CVE-2018-3646 and https://www.kernel.org/doc/html/latest/admin-guide/l1tf.html for details.\n"
#define L1TF_MSG_L1D_BS "L1TF CPU bug present and virtualization mitigation disabled, data leak possible. See CVE-2018-3646 and https://www.kernel.org/doc/html/latest/admin-guide/l1tf.html for details.\n"

static int vmx_vm_init_bs(struct kvm *kvm)
{
	spin_lock_init(&to_kvm_vmx_bs(kvm)->ept_pointer_lock);

	if (!ple_gap_bs)
		kvm->arch.pause_in_guest = true;

	if (boot_cpu_has(X86_BUG_L1TF) && enable_ept_bs) {
		switch (l1tf_mitigation) {
		case L1TF_MITIGATION_OFF:
		case L1TF_MITIGATION_FLUSH_NOWARN:
			/* 'I explicitly don't care' is set */
			break;
		case L1TF_MITIGATION_FLUSH:
		case L1TF_MITIGATION_FLUSH_NOSMT:
		case L1TF_MITIGATION_FULL:
			/*
			 * Warn upon starting the first VM in a potentially
			 * insecure environment.
			 */
			if (sched_smt_active())
				pr_warn_once(L1TF_MSG_SMT_BS);
			if (l1tf_vmx_mitigation == VMENTER_L1D_FLUSH_NEVER)
				pr_warn_once(L1TF_MSG_L1D_BS);
			break;
		case L1TF_MITIGATION_FULL_FORCE:
			/* Flush is enforced */
			break;
		}
	}

	return 0;
}

static __init int cpu_has_kvm_support_bs(void)
{
	return cpu_has_vmx_bs();
}

static __init int vmx_disabled_by_bios_bs(void)
{
	u64 msr;

	rdmsrl(MSR_IA32_FEATURE_CONTROL, msr);
	if (msr & FEATURE_CONTROL_LOCKED) {
		/* launched w/ TXT and VMX disabled */
		if (!(msr & FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX)
				&& tboot_enabled())
			return 1;
		/* launched w/o TXT and VMX only enabled w/ TXT */
		if (!(msr & FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX)
			&& (msr & FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX)
			&& !tboot_enabled()) {
			printk("kvm: disable TXT in the BIOS or "
			       "activate TXT before enabling KVM\n");
			return 1;
		}
	}

	return 0;
}

static __init int adjust_vmx_controls_bs(u32 ctl_min, u32 ctl_opt,
			u32 msr, u32 *result)
{
	u32 vmx_msr_low, vmx_msr_high;
	u32 ctl = ctl_min | ctl_opt;

	rdmsr(msr, vmx_msr_low, vmx_msr_high);

	ctl &= vmx_msr_high; /* bit == 0 in high word ==> must be zero */
	ctl |= vmx_msr_low;  /* bit == 1 in low word ==> must be one */

	/* Ensure minimum (required) set of control bits are supported. */
	if (ctl_min & ~ctl)
		return -EIO;

	*result = ctl;
	return 0;
}

static __init int setup_vmcs_config_bs(struct vmcs_config *vmcs_conf,
					struct vmx_capability *vmx_cap)
{
	u32 vmx_msr_low, vmx_msr_high;
	u32 min, opt, min2, opt2;
	u32 _pin_based_exec_control = 0;
	u32 _cpu_based_exec_control = 0;
	u32 _cpu_based_2nd_exec_control = 0;
	u32 _vmexit_control = 0;
	u32 _vmentry_control = 0;

	memset(vmcs_conf, 0, sizeof(*vmcs_conf));
	min =	CPU_BASED_HLT_EXITING |
		CPU_BASED_CR3_LOAD_EXITING |
		CPU_BASED_CR3_STORE_EXITING |
		CPU_BASED_UNCOND_IO_EXITING |
		CPU_BASED_MOV_DR_EXITING |
		CPU_BASED_USE_TSC_OFFSETING |
		CPU_BASED_MWAIT_EXITING | 
		CPU_BASED_MONITOR_EXITING |
		CPU_BASED_INVLPG_EXITING |
		CPU_BASED_RDPMC_EXITING;

	opt =	CPU_BASED_TPR_SHADOW |
		CPU_BASED_USE_MSR_BITMAPS |
		CPU_BASED_ACTIVATE_SECONDARY_CONTROLS;

	if (adjust_vmx_controls_bs(min, opt, MSR_IA32_VMX_PROCBASED_CTLS,
				&_cpu_based_exec_control) < 0)
		return -EIO;
	if (_cpu_based_exec_control & CPU_BASED_ACTIVATE_SECONDARY_CONTROLS) {
		min2 = 0;
		opt2 =	SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES |
			SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE |
			SECONDARY_EXEC_WBINVD_EXITING |
			SECONDARY_EXEC_ENABLE_VPID |
			SECONDARY_EXEC_ENABLE_EPT |
			SECONDARY_EXEC_UNRESTRICTED_GUEST |
			SECONDARY_EXEC_PAUSE_LOOP_EXITING |
			SECONDARY_EXEC_DESC |
			SECONDARY_EXEC_RDTSCP |
			SECONDARY_EXEC_ENABLE_INVPCID |
			SECONDARY_EXEC_APIC_REGISTER_VIRT |
			SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY |
			SECONDARY_EXEC_SHADOW_VMCS |
			SECONDARY_EXEC_XSAVES |
			SECONDARY_EXEC_RDSEED_EXITING |
			SECONDARY_EXEC_RDRAND_EXITING |
			SECONDARY_EXEC_ENABLE_PML |
			SECONDARY_EXEC_TSC_SCALING |
			SECONDARY_EXEC_PT_USE_GPA |
			SECONDARY_EXEC_PT_CONCEAL_VMX |
			SECONDARY_EXEC_ENABLE_VMFUNC |
			SECONDARY_EXEC_ENCLS_EXITING;
		if (adjust_vmx_controls_bs(min2, opt2,
				MSR_IA32_VMX_PROCBASED_CTLS2,
				&_cpu_based_2nd_exec_control) < 0)
			return -EIO;
	}
	if (!(_cpu_based_exec_control & CPU_BASED_TPR_SHADOW))
		_cpu_based_2nd_exec_control &= ~(
				SECONDARY_EXEC_APIC_REGISTER_VIRT |
				SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE |
				SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY);

	rdmsr_safe(MSR_IA32_VMX_EPT_VPID_CAP,
			&vmx_cap->ept, &vmx_cap->vpid);

	if (_cpu_based_2nd_exec_control & SECONDARY_EXEC_ENABLE_EPT) {
		/* CR3 accesses and invlpg don't need to cause VM Exits when
		 * enabled */
		_cpu_based_exec_control &= ~(CPU_BASED_CR3_LOAD_EXITING |
					     CPU_BASED_CR3_STORE_EXITING |
					     CPU_BASED_INVLPG_EXITING);
	} else if (vmx_cap->ept) {
		vmx_cap->ept = 0;
		printk("EPT CAP should not exist if not support "
			"1-setting enable EPT VM-execution control\n");
	}
	if (!(_cpu_based_2nd_exec_control & SECONDARY_EXEC_ENABLE_VPID) &&
		vmx_cap->vpid) {
		vmx_cap->vpid = 0;
		printk("VPID CAP should not exist if not support "
			"1-setting enable VPID VM-execution control\n");
	}

	min = VM_EXIT_SAVE_DEBUG_CONTROLS | VM_EXIT_ACK_INTR_ON_EXIT;

	opt =	VM_EXIT_LOAD_IA32_PERF_GLOBAL_CTRL |
		VM_EXIT_SAVE_IA32_PAT |
		VM_EXIT_LOAD_IA32_PAT |
		VM_EXIT_LOAD_IA32_EFER |
		VM_EXIT_CLEAR_BNDCFGS |
		VM_EXIT_PT_CONCEAL_PIP |
		VM_EXIT_CLEAR_IA32_RTIT_CTL;
	if (adjust_vmx_controls_bs(min, opt, MSR_IA32_VMX_EXIT_CTLS,
				&_vmexit_control) < 0)
		return -EIO;

	min = PIN_BASED_EXT_INTR_MASK | PIN_BASED_NMI_EXITING;
	opt = PIN_BASED_VIRTUAL_NMIS | PIN_BASED_POSTED_INTR |
		PIN_BASED_VMX_PREEMPTION_TIMER;
	if (adjust_vmx_controls_bs(min, opt, MSR_IA32_VMX_PINBASED_CTLS,
			&_pin_based_exec_control) < 0)
		return -EIO;

	if (cpu_has_broken_vmx_preemption_timer_bs())
		_pin_based_exec_control &= ~PIN_BASED_VMX_PREEMPTION_TIMER;
	if (!(_cpu_based_2nd_exec_control &
			SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY))
		_pin_based_exec_control &= ~PIN_BASED_POSTED_INTR;

	min = VM_ENTRY_LOAD_DEBUG_CONTROLS;
	opt =	VM_ENTRY_LOAD_IA32_PERF_GLOBAL_CTRL |
		VM_ENTRY_LOAD_IA32_PAT |
		VM_ENTRY_LOAD_IA32_EFER |
		VM_ENTRY_LOAD_BNDCFGS |
		VM_ENTRY_PT_CONCEAL_PIP |
		VM_ENTRY_LOAD_IA32_RTIT_CTL;
	if (adjust_vmx_controls_bs(min, opt, MSR_IA32_VMX_ENTRY_CTLS,
						&_vmentry_control) < 0)
		return -EIO;

	/*
	 * Some cpus support VM_{ENTRY,EXIT}_IA32_PERF_GLOBAL_CTRL but they
	 * can't be used due to an errata where VM Exit may incorrectly clear
	 * IA32_PERF_GLOBAL_CTRL[34:32].  Workaround the errata by using the
	 * MSR load mechanism to switch IA32_PERF_GLOBAL_CTRL.
	 */
	if (boot_cpu_data.x86 == 0x6) {
		switch (boot_cpu_data.x86_model) {
		case 26: /* AAK155 */
		case 30: /* AAP115 */
		case 37: /* AAT100 */
		case 44: /* BC86,AAY89,BD102 */
		case 46: /* BA97 */
			_vmentry_control &= 
				~VM_ENTRY_LOAD_IA32_PERF_GLOBAL_CTRL;
			_vmexit_control &= ~VM_EXIT_LOAD_IA32_PERF_GLOBAL_CTRL;
			printk("kvm: VM_EXIT_LOAD_IA32_PERF_GLOBAL_CTRL "
				"does not work properly. Using workaround\n");
			break;
		default:
			break;
		}
	}

	rdmsr(MSR_IA32_VMX_BASIC, vmx_msr_low, vmx_msr_high);

	/* IA-32 SDM Vol 3B: VMCS size is never greater than 4kB. */
	if ((vmx_msr_high & 0x1fff) > PAGE_SIZE)
		return -EIO;

        /* Require Write-Back (WB) memory type for VMCS accesses. */
        if (((vmx_msr_high >> 18) & 15) != 6)
                return -EIO;

	vmcs_conf->size = vmx_msr_high & 0x1fff;
	vmcs_conf->order = get_order(vmcs_conf->size);
	vmcs_conf->basic_cap = vmx_msr_high & ~0x1fff;

	vmcs_conf->revision_id = vmx_msr_low;

	vmcs_conf->pin_based_exec_ctrl = _pin_based_exec_control;
	vmcs_conf->cpu_based_exec_ctrl = _cpu_based_exec_control;
	vmcs_conf->cpu_based_2nd_exec_ctrl = _cpu_based_2nd_exec_control;
	vmcs_conf->vmexit_ctrl = _vmexit_control;
	vmcs_conf->vmentry_ctrl = _vmentry_control;

	if (static_branch_unlikely(&enable_evmcs_bs))
		evmcs_sanitize_exec_ctrls_bs(vmcs_conf);

	return 0;
}

static void ept_set_mmio_spte_mask_bs(void)
{
	/*
	 * EPT Misconfigurations can be generated if the value of bits 2:0
	 * of an EPT paging-structure entry is 110b (write/execte)
	 */
	kvm_mmu_set_mmio_spte_mask_bs(VMX_EPT_RWX_MASK,
			VMX_EPT_MISCONFIG_WX_VALUE);
}

static void vmx_enable_tdp_bs(void)
{
	kvm_mmu_set_mask_ptes_bs(VMX_EPT_READABLE_MASK,
		enable_ept_ad_bits_bs ? VMX_EPT_ACCESS_BIT : 0ull,
		enable_ept_ad_bits_bs ? VMX_EPT_DIRTY_BIT : 0ull,
		0ull, VMX_EPT_EXECUTABLE_MASK,
		cpu_has_vmx_ept_execute_only_bs() ? 0ull : 
		VMX_EPT_READABLE_MASK, VMX_EPT_RWX_MASK, 0ull);

	ept_set_mmio_spte_mask_bs();
	kvm_enable_tdp_bs();
}

/*
 * Handler for POSTED_INTERRUPT_WAKEUP_VECTOR
 */
static void wakeup_handler_bs(void)
{
	BS_DUP();
}

struct vmcs *alloc_vmcs_cpu_bs(bool shadow, int cpu)
{
	int node = cpu_to_node(cpu);
	struct page *pages;
	struct vmcs *vmcs;

	pages = __alloc_pages_node(node, GFP_KERNEL, vmcs_config_bs.order);
	if (!pages)
		return NULL;
	vmcs = page_address(pages);
	memset(vmcs, 0, vmcs_config_bs.size);

	/* KVM supports Enlightened VMCS v1 only */
	if (static_branch_unlikely(&enable_evmcs_bs))
		vmcs->hdr.revision_id = KVM_EVMCS_VERSION;
	else
		vmcs->hdr.revision_id = vmcs_config_bs.revision_id;

	if (shadow)
		vmcs->hdr.shadow_vmcs = 1;
	return vmcs;
}

void free_vmcs_bs(struct vmcs *vmcs)
{
	free_pages((unsigned long)vmcs, vmcs_config_bs.order);
}

static void free_kvm_area_bs(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		free_vmcs_bs(per_cpu(vmxarea_bs, cpu));
		per_cpu(vmxarea_bs, cpu) = NULL;
	}
}

static __init int alloc_kvm_area_bs(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		struct vmcs *vmcs;

		vmcs = alloc_vmcs_cpu_bs(false, cpu);
		if (!vmcs) {
			free_kvm_area_bs();
			return -ENOMEM;
		}

		/*
		 * When eVMCS is enabled, alloc_vmcs_cpu() sets
		 * vmcs->revision_id to KVM_EVMCS_VERSION instead of
		 * revision_id reported by MSR_IA32_VMX_BASIC.
		 *
		 * However, even though not explicitly documented by
		 * TLFS, VMXArea passed as VMXON argument should
		 * still be marked with revision_id reported by
		 * physical CPU.
		 */
		if (static_branch_unlikely(&enable_evmcs_bs))
			vmcs->hdr.revision_id = vmcs_config_bs.revision_id;

		per_cpu(vmxarea_bs, cpu) = vmcs;
	}
	return 0;
}

static int handle_exception_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_external_interrupt_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_triple_fault_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_nmi_window_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_io_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_cr_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_dr_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_cpuid_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_rdmsr_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_wrmsr_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_halt_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invd_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invlpg_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_rdpmc_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmcall_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_tpr_below_threshold_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_apic_access_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_apic_write_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_wbinvd_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_xsetbv_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_task_switch_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_machine_check_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_desc_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_ept_violation_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_ept_misconfig_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_pause_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_mwait_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_monitor_trap_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_monitor_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmx_instruction_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invalid_op_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_xsaves_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_xrstors_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_pml_full_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invpcid_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_preemption_timer_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_encls_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_interrupt_window_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_apic_eoi_induced_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

/*
 * The exit handlers return 1 if the exit was handled fully and guest execution
 * may resume.  Otherwise they set the kvm_run parameter to indicate what needs
 * to be done to userspace and return 0.
 */
static int (*kvm_vmx_exit_handlers_bs[])(struct kvm_vcpu *vcpu) = {
	[EXIT_REASON_EXCEPTION_NMI]		= handle_exception_bs,
	[EXIT_REASON_EXTERNAL_INTERRUPT]	= handle_external_interrupt_bs,
	[EXIT_REASON_TRIPLE_FAULT]		= handle_triple_fault_bs,
	[EXIT_REASON_NMI_WINDOW]		= handle_nmi_window_bs,
	[EXIT_REASON_IO_INSTRUCTION]		= handle_io_bs,
	[EXIT_REASON_CR_ACCESS]			= handle_cr_bs,
	[EXIT_REASON_DR_ACCESS]			= handle_dr_bs,
	[EXIT_REASON_CPUID]			= handle_cpuid_bs,
	[EXIT_REASON_MSR_READ]			= handle_rdmsr_bs,
	[EXIT_REASON_MSR_WRITE]			= handle_wrmsr_bs,
	[EXIT_REASON_PENDING_INTERRUPT]		= handle_interrupt_window_bs,
	[EXIT_REASON_HLT]			= handle_halt_bs,
	[EXIT_REASON_INVD]			= handle_invd_bs,
	[EXIT_REASON_INVLPG]			= handle_invlpg_bs,
	[EXIT_REASON_RDPMC]			= handle_rdpmc_bs,
	[EXIT_REASON_VMCALL]			= handle_vmcall_bs,
	[EXIT_REASON_VMCLEAR]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMLAUNCH]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMPTRLD]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMPTRST]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMREAD]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMRESUME]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMWRITE]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMOFF]			= handle_vmx_instruction_bs,
	[EXIT_REASON_VMON]			= handle_vmx_instruction_bs,
	[EXIT_REASON_TPR_BELOW_THRESHOLD]	= handle_tpr_below_threshold_bs,
	[EXIT_REASON_APIC_ACCESS]		= handle_apic_access_bs,
	[EXIT_REASON_APIC_WRITE]		= handle_apic_write_bs,
	[EXIT_REASON_EOI_INDUCED]		= handle_apic_eoi_induced_bs,
	[EXIT_REASON_WBINVD]			= handle_wbinvd_bs,
	[EXIT_REASON_XSETBV]			= handle_xsetbv_bs,
	[EXIT_REASON_TASK_SWITCH]		= handle_task_switch_bs,
	[EXIT_REASON_MCE_DURING_VMENTRY]	= handle_machine_check_bs,
	[EXIT_REASON_GDTR_IDTR]			= handle_desc_bs,
	[EXIT_REASON_LDTR_TR]			= handle_desc_bs,
	[EXIT_REASON_EPT_VIOLATION]		= handle_ept_violation_bs,
	[EXIT_REASON_EPT_MISCONFIG]		= handle_ept_misconfig_bs,
	[EXIT_REASON_PAUSE_INSTRUCTION]		= handle_pause_bs,
	[EXIT_REASON_MWAIT_INSTRUCTION]		= handle_mwait_bs,
	[EXIT_REASON_MONITOR_TRAP_FLAG]		= handle_monitor_trap_bs,
	[EXIT_REASON_MONITOR_INSTRUCTION]	= handle_monitor_bs,
	[EXIT_REASON_INVEPT]			= handle_vmx_instruction_bs,
	[EXIT_REASON_INVVPID]			= handle_vmx_instruction_bs,
	[EXIT_REASON_RDRAND]			= handle_invalid_op_bs,
	[EXIT_REASON_RDSEED]			= handle_invalid_op_bs,
	[EXIT_REASON_XSAVES]			= handle_xsaves_bs,
	[EXIT_REASON_XRSTORS]			= handle_xrstors_bs,
	[EXIT_REASON_PML_FULL]			= handle_pml_full_bs,
	[EXIT_REASON_INVPCID]			= handle_invpcid_bs,
	[EXIT_REASON_VMFUNC]			= handle_vmx_instruction_bs,
	[EXIT_REASON_PREEMPTION_TIMER]		= handle_preemption_timer_bs,
	[EXIT_REASON_ENCLS]			= handle_encls_bs,
};

static __init int hardware_setup_bs(void)
{
	unsigned long host_bndcfgs;
	int r, i;

	rdmsrl_safe(MSR_EFER, &host_efer_bs);

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index_bs); ++i)
		kvm_define_shared_msr_bs(i, vmx_msr_index_bs[i]);

	if (setup_vmcs_config_bs(&vmcs_config_bs, &vmx_capability_bs) < 0)
		return -EIO;

	if (boot_cpu_has(X86_FEATURE_NX))
		kvm_enable_efer_bits_bs(EFER_NX);

	if (boot_cpu_has(X86_FEATURE_MPX)) {
		rdmsrl(MSR_IA32_BNDCFGS, host_bndcfgs);
		WARN_ONCE(host_bndcfgs, "KVM: BNDCFGS in host will be lost");
	}

	if (boot_cpu_has(X86_FEATURE_XSAVES))
		rdmsrl(MSR_IA32_XSS, host_xss_bs);

	if (!cpu_has_vmx_vpid_bs() || !cpu_has_vmx_invvpid_bs() ||
	    			!(cpu_has_vmx_invvpid_single_bs() || 
				cpu_has_vmx_invvpid_global_bs()))
		enable_vpid_bs = 0;

	if (!cpu_has_vmx_ept_bs() ||
	    !cpu_has_vmx_ept_4levels_bs() ||
	    !cpu_has_vmx_invept_global_bs())
		enable_ept_bs = 0;

	if (!cpu_has_vmx_ept_ad_bits_bs() || !enable_ept_bs)
		enable_ept_ad_bits_bs = 0;

	if (!cpu_has_vmx_unrestricted_guest_bs() || !enable_ept_bs)
		enable_unrestricted_guest_bs = 0;

	if (!cpu_has_vmx_flexpriority_bs())
		flexpriority_enabled_bs = 0;

	if (!cpu_has_virtual_nmis_bs())
		enable_vnmi_bs = 0;

	/*
	 * set_apic_access_page_addr() is used to reload apic access
	 * page upon invalidation.  No need to do anything if not
	 * using the APIC_ACCESS_ADDR VMCS field.
	 */
	if (!flexpriority_enabled_bs)
		kvm_x86_ops_bs->set_apic_access_page_addr = NULL;

	if (!cpu_has_vmx_tpr_shadow_bs())
		kvm_x86_ops_bs->update_cr8_intercept = NULL;

	if (enable_ept_bs && !cpu_has_vmx_ept_2m_page_bs())
		kvm_disable_largepages_bs();

	if (!cpu_has_vmx_ple_bs()) {
		ple_gap_bs = 0;
		ple_window_bs = 0;
		ple_window_grow_bs = 0;
		ple_window_max_bs = 0;
		ple_window_shrink_bs = 0;
	}

	if (!cpu_has_vmx_apicv_bs()) {
		enable_apicv_bs = 0;
		kvm_x86_ops_bs->sync_pir_to_irr = NULL;
	}

	if (cpu_has_vmx_tsc_scaling_bs()) {
		kvm_has_tsc_control_bs = true;
		kvm_max_tsc_scaling_ratio_bs = KVM_VMX_TSC_MULTIPLIER_MAX;
		kvm_tsc_scaling_ratio_frac_bits_bs = 48;
	}

	set_bit(0, vmx_vpid_bitmap_bs); /* 0 is reserved for host */

	if (enable_ept_bs)
		vmx_enable_tdp_bs();
	else
		kvm_disable_tdp_bs();

	/*
	 * Only enable PML when hardware supports PML feature, and both EPT
	 * and EPT A/D bit features are enabled -- PML depends on them to work.
	 */
	if (!enable_ept_bs || !enable_ept_ad_bits_bs || !cpu_has_vmx_pml_bs())
		enable_pml_bs = 0;

	if (!enable_pml_bs) {
		kvm_x86_ops_bs->slot_enable_log_dirty = NULL;
		kvm_x86_ops_bs->slot_disable_log_dirty = NULL;
		kvm_x86_ops_bs->flush_log_dirty = NULL;
		kvm_x86_ops_bs->enable_log_dirty_pt_masked = NULL;
	}

	if (!cpu_has_vmx_preemption_timer_bs())
		kvm_x86_ops_bs->request_immediate_exit = 
				__kvm_request_immediate_exit_bs;

	if (cpu_has_vmx_preemption_timer_bs() && enable_preemption_timer_bs) {
		u64 vmx_msr;

		rdmsrl(MSR_IA32_VMX_MISC, vmx_msr);
		cpu_preemption_timer_multi_bs =
			vmx_msr & VMX_MISC_PREEMPTION_TIMER_RATE_MASK;
	} else {
		kvm_x86_ops_bs->set_hv_timer = NULL;
		kvm_x86_ops_bs->cancel_hv_timer = NULL;
	}

	kvm_set_posted_intr_wakeup_handler(wakeup_handler_bs);

	kvm_mce_cap_supported_bs |= MCG_LMCE_P;

	if (pt_mode_bs != PT_MODE_SYSTEM && pt_mode_bs != PT_MODE_HOST_GUEST)
		return -EINVAL;
	if (!enable_ept_bs || !cpu_has_vmx_intel_pt_bs())
		pt_mode_bs = PT_MODE_SYSTEM;

	if (nested_bs) {
		nested_vmx_setup_ctls_msrs_bs(&vmcs_config_bs.nested,
				vmx_capability_bs.ept, enable_apicv_bs);

		r = nested_vmx_hardware_setup_bs(kvm_vmx_exit_handlers_bs);
		if (r)
			return r;
	}

	r = alloc_kvm_area_bs();
	if (r)
		BS_DUP();
	return r;
}

static void __init vmx_check_processor_compat_bs(void *rtn)
{
	struct vmcs_config vmcs_conf;
	struct vmx_capability vmx_cap;

	*(int *)rtn = 0;
	if (setup_vmcs_config_bs(&vmcs_conf, &vmx_cap) < 0)
		*(int *)rtn = -EIO;
	if (nested_bs)
		nested_vmx_setup_ctls_msrs_bs(&vmcs_conf.nested, vmx_cap.ept,
				enable_apicv_bs);
	if (memcmp(&vmcs_conf, &vmcs_conf, sizeof(struct vmcs_config)) != 0) {
		printk("kvm: CPU %d feature inconsistency!\n", 
						smp_processor_id());
		*(int *)rtn = -EIO;
	}
}

static bool vmx_rdtscp_supported_bs(void)
{
	return cpu_has_vmx_rdtscp_bs();
}

static bool vmx_has_emulated_msr_bs(int index)
{
	switch (index) {
	case MSR_IA32_SMBASE:
		/*
		 * We cannot do SMM unless we can run the guest in big
		 * real mode.
		 */
		return enable_unrestricted_guest_bs || 
			emulate_invalid_guest_state_bs;
	case MSR_AMD64_VIRT_SPEC_CTRL:
		/* This is AMD only.  */
		return false;
	default:
		return true;
	}
}

static int vmx_get_msr_feature_bs(struct kvm_msr_entry *msr)
{
	switch (msr->index) {
	case MSR_IA32_VMX_BASIC ... MSR_IA32_VMX_VMFUNC:
		if (!nested_bs)
			return 1;
		return vmx_get_vmx_msr_bs(&vmcs_config_bs.nested, 
					msr->index, &msr->data);
	default:
		return 1;
	}

	return 0;
}

static int vmx_setup_l1d_flush_bs(enum vmx_l1d_flush_state l1tf)
{
	struct page *page;
	unsigned int i;

	if (!enable_ept_bs) {
		l1tf_vmx_mitigation = VMENTER_L1D_FLUSH_EPT_DISABLED;
		return 0;
	}

	if (boot_cpu_has(X86_FEATURE_ARCH_CAPABILITIES)) {
		u64 msr;

		rdmsrl(MSR_IA32_ARCH_CAPABILITIES, msr);
		if (msr & ARCH_CAP_SKIP_VMENTRY_L1DFLUSH) {
			l1tf_vmx_mitigation = VMENTER_L1D_FLUSH_NOT_REQUIRED;
			return 0;
		}
	}

	/* If set to auto use the default l1tf mitigation method */
	if (l1tf == VMENTER_L1D_FLUSH_AUTO) {
		switch (l1tf_mitigation) {
		case L1TF_MITIGATION_OFF:
			l1tf = VMENTER_L1D_FLUSH_NEVER;
			break;
		case L1TF_MITIGATION_FLUSH_NOWARN:
		case L1TF_MITIGATION_FLUSH:
		case L1TF_MITIGATION_FLUSH_NOSMT:
			l1tf = VMENTER_L1D_FLUSH_COND;
			break;
		case L1TF_MITIGATION_FULL:
		case L1TF_MITIGATION_FULL_FORCE:
			l1tf = VMENTER_L1D_FLUSH_ALWAYS;
			break;
		}
	} else if (l1tf_mitigation == L1TF_MITIGATION_FULL_FORCE) {
		l1tf = VMENTER_L1D_FLUSH_ALWAYS;
	}

	if (l1tf != VMENTER_L1D_FLUSH_NEVER && !vmx_l1d_flush_pages_bs &&
				!boot_cpu_has(X86_FEATURE_FLUSH_L1D)) {
		page = alloc_pages(GFP_KERNEL, L1D_CACHE_ORDER);
		if (!page)
			return -ENOMEM;
		vmx_l1d_flush_pages_bs = page_address(page);

		/*
		 * Initialize each page with a different pattern in
		 * order to protect against KSM in the nested
		 * virtualization case.
		 */
		for (i = 0; i < 1u << L1D_CACHE_ORDER; ++i) {
			memset(vmx_l1d_flush_pages_bs + i * PAGE_SIZE, i + 1,
			PAGE_SIZE);
		}
	}

	l1tf_vmx_mitigation = l1tf;

	if (l1tf != VMENTER_L1D_FLUSH_NEVER)
		static_branch_enable(&vmx_l1d_should_flush_bs);
	else
		static_branch_disable(&vmx_l1d_should_flush_bs);

	if (l1tf == VMENTER_L1D_FLUSH_COND)
		static_branch_enable(&vmx_l1d_flush_cond_bs);
	else
		static_branch_disable(&vmx_l1d_flush_cond_bs);
	return 0;
}

/*
 * This bitmap is used to indicate whether the vmclear
 * operation is enabled on all cpus. All disabled by
 * default.
 */
static cpumask_t crash_vmclear_enabled_bitmap_bs = CPU_MASK_NONE;

static inline int crash_local_vmclear_enabled_bs(int cpu)
{
	return cpumask_test_cpu(cpu, &crash_vmclear_enabled_bitmap_bs);
}

static inline void crash_enable_local_vmclear_bs(int cpu)
{
	cpumask_set_cpu(cpu, &crash_vmclear_enabled_bitmap_bs);
}

static void crash_vmclear_local_loaded_vmcss_bs(void)
{
	int cpu = raw_smp_processor_id();
	struct loaded_vmcs *v;

	if (!crash_local_vmclear_enabled_bs(cpu))
		return;

	list_for_each_entry(v, &per_cpu(loaded_vmcss_on_cpu_bs, cpu),
		loaded_vmcss_on_cpu_link)
		vmcs_clear_bs(v->vmcs);
}

static struct kvm *vmx_vm_alloc_bs(void)
{
	struct kvm_vmx *kvm_vmx = vzalloc(sizeof(struct kvm_vmx));
	return &kvm_vmx->kvm;
}

static void kvm_cpu_vmxon_bs(u64 addr)
{
	cr4_set_bits(X86_CR4_VMXE);
	intel_pt_handle_vmx(1);

	asm volatile ("vmxon %0" : : "m" (addr));
}

static int hardware_enable_bs(void)
{
	int cpu = raw_smp_processor_id();
	u64 phys_addr = __pa(per_cpu(vmxarea_bs, cpu));
	u64 old, test_bits;

	if (cr4_read_shadow() & X86_CR4_VMXE)
		return -EBUSY;

	/*
	 * This can happen if we hot-added a CPU but failed to allocate
	 * VP assist page for it.
	 */
	if (static_branch_unlikely(&enable_evmcs_bs) &&
				!hv_get_vp_assist_page(cpu))
		return -EFAULT;

	INIT_LIST_HEAD(&per_cpu(loaded_vmcss_on_cpu_bs, cpu));
	INIT_LIST_HEAD(&per_cpu(blocked_vcpu_on_cpu_bs, cpu));
	spin_lock_init(&per_cpu(blocked_vcpu_on_cpu_lock_bs, cpu));

	/*
	 * Now we can enable the vmclear operation in kdump
	 * since the loaded_vmcss_on_cpu list on this cpu
	 * has been initialized.
	 *
	 * Though the cpu is not in VMX operation now, there
	 * is no problem to enable the vmclear operation
	 * for the loaded_vmcss_on_cpu list is empty!
	 */
	crash_enable_local_vmclear_bs(cpu);

	rdmsrl(MSR_IA32_FEATURE_CONTROL, old);

	test_bits = FEATURE_CONTROL_LOCKED;
	test_bits |= FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX;
	if (tboot_enabled())
		test_bits |= FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX;

	if ((old & test_bits) != test_bits) {
		/* enable and lock */
		wrmsrl(MSR_IA32_FEATURE_CONTROL, old | test_bits);
	}
	kvm_cpu_vmxon_bs(phys_addr);
	if (enable_ept_bs)
		ept_sync_global_bs();

	return 0;
}

static struct kvm_x86_ops vmx_x86_ops_bs __ro_after_init = {
	.cpu_has_kvm_support = cpu_has_kvm_support_bs,
	.disabled_by_bios = vmx_disabled_by_bios_bs,
	.hardware_setup = hardware_setup_bs,
	.check_processor_compatibility = vmx_check_processor_compat_bs,
	.rdtscp_supported = vmx_rdtscp_supported_bs,
	.has_emulated_msr = vmx_has_emulated_msr_bs,
	.get_msr_feature = vmx_get_msr_feature_bs,
	.hardware_enable = hardware_enable_bs,

	.vm_init = vmx_vm_init_bs,
	.vm_alloc = vmx_vm_alloc_bs,
};

static int __init vmx_init_bs(void)
{
	int r;

	BS_DONE();
	r = kvm_init_bs(&vmx_x86_ops_bs, sizeof(struct vcpu_vmx),
			__alignof__(struct vcpu_vmx), THIS_MODULE);
	if (r)
		return r;

	/*
	 * Must be called after kvm_init() so enable_ept is properly set
	 * up. Hand the parameter mitigation value in which was stored in
	 * the pre module init parser. If no parameter was given, it will
	 * contain 'auto' which will be turned into the default 'cond'
	 * mitigation mode.
	 */
	if (boot_cpu_has(X86_BUG_L1TF)) {
		r = vmx_setup_l1d_flush_bs(vmentry_l1d_flush_param_bs);
		if (r) {
			BS_DUP();
			return r;
		}
	}

#ifdef CONFIG_KEXEC_CORE
	rcu_assign_pointer(crash_vmclear_loaded_vmcss,
			   crash_vmclear_local_loaded_vmcss_bs);
#endif

	vmx_check_vmcs12_offsets_bs();
	return 0;
}

static void __exit vmx_exit_bs(void)
{
}

module_init(vmx_init_bs);
module_exit(vmx_exit_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("vmx Project");
