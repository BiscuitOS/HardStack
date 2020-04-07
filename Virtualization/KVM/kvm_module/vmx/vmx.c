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

#include "kvm/internal.h"
#include "kvm/vmx.h"
#include "kvm/virtext.h"
#include "kvm/capabilities.h"

u64 host_efer_bs;

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

static int vmx_vm_init_bs(struct kvm *kvm)
{
	BS_DUP();
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

static __init int hardware_setup_bs(void)
{
	int i;

	rdmsrl_safe(MSR_EFER, &host_efer_bs);

	for (i = 0; i < ARRAY_SIZE(vmx_msr_index_bs); ++i)
		kvm_define_shared_msr_bs(i, vmx_msr_index_bs[i]);

	if (setup_vmcs_config_bs(&vmcs_config_bs, &vmx_capability_bs) < 0)
		return -EIO;

	return 0;
}

static struct kvm_x86_ops vmx_x86_ops_bs __ro_after_init = {
	.cpu_has_kvm_support = cpu_has_kvm_support_bs,
	.disabled_by_bios = vmx_disabled_by_bios_bs,
	.hardware_setup = hardware_setup_bs,
	.vm_init = vmx_vm_init_bs,
};

static int __init vmx_init_bs(void)
{
	int r;

	r = kvm_init_bs(&vmx_x86_ops_bs, sizeof(struct vcpu_vmx),
			__alignof__(struct vcpu_vmx), THIS_MODULE);
	if (r)
		return r;

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
