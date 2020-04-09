/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KVM_X86_VMX_CAPS_H
#define __KVM_X86_VMX_CAPS_H

#include <linux/types.h>
#include <asm/vmx.h>

#define PT_MODE_SYSTEM		0
#define PT_MODE_HOST_GUEST	1

struct nested_vmx_msrs {
	/*
	 * We only store the "true" versions of the VMX capability MSRs. We
	 * generate the "non-true" versions by setting the must-be-1 bits
	 * according to the SDM.
	 */
	u32 procbased_ctls_low;
	u32 procbased_ctls_high;
	u32 secondary_ctls_low;
	u32 secondary_ctls_high;
	u32 pinbased_ctls_low;
	u32 pinbased_ctls_high;
	u32 exit_ctls_low;
	u32 exit_ctls_high;
	u32 entry_ctls_low;
	u32 entry_ctls_high;
	u32 misc_low;
	u32 misc_high;
	u32 ept_caps;
	u32 vpid_caps;
	u64 basic;
	u64 cr0_fixed0;
	u64 cr0_fixed1;
	u64 cr4_fixed0;
	u64 cr4_fixed1;
	u64 vmcs_enum;
	u64 vmfunc_controls;
};

struct vmcs_config {
	int size;
	int order;
	u32 basic_cap;
	u32 revision_id;
	u32 pin_based_exec_ctrl;
	u32 cpu_based_exec_ctrl;
	u32 cpu_based_2nd_exec_ctrl;
	u32 vmexit_ctrl;
	u32 vmentry_ctrl;
	struct nested_vmx_msrs nested;
};

struct vmx_capability {
	u32 ept;
	u32 vpid;
};

extern struct vmcs_config vmcs_config_bs;
extern struct vmx_capability vmx_capability_bs;

static inline bool cpu_has_vmx_basic_inout_bs(void)
{
	return (((u64)vmcs_config_bs.basic_cap << 32) & VMX_BASIC_INOUT);
}

static inline bool cpu_has_vmx_vpid_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
			SECONDARY_EXEC_ENABLE_VPID;
}

static inline bool cpu_has_vmx_ept_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
			SECONDARY_EXEC_ENABLE_EPT;
}

static inline bool cpu_has_vmx_unrestricted_guest_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
			SECONDARY_EXEC_UNRESTRICTED_GUEST;
}

static inline bool cpu_has_vmx_tpr_shadow_bs(void)
{
	return vmcs_config_bs.cpu_based_exec_ctrl & CPU_BASED_TPR_SHADOW;
}

static inline bool cpu_has_vmx_virtualize_apic_accesses_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES;
}

static inline bool cpu_has_vmx_shadow_vmcs_bs(void)
{
	u64 vmx_msr;

	/* Check if the cpu supports writting r/o exit information fields */
	rdmsrl(MSR_IA32_VMX_MISC, vmx_msr);
	if (!(vmx_msr & MSR_IA32_VMX_MISC_VMWRITE_SHADOW_RO_FIELDS))
		return false;

	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_SHADOW_VMCS;
}

static inline bool cpu_has_virtual_nmis_bs(void)
{
	return vmcs_config_bs.pin_based_exec_ctrl & PIN_BASED_VIRTUAL_NMIS;
}

static inline bool cpu_has_vmx_vmfunc_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_ENABLE_VMFUNC;
}

static inline bool cpu_has_vmx_ple_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_PAUSE_LOOP_EXITING;
}

static inline bool cpu_has_vmx_apic_register_virt_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_APIC_REGISTER_VIRT;
}

static inline bool cpu_has_vmx_virtual_intr_delivery_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY;
}

static inline bool cpu_has_vmx_rdtscp_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_RDTSCP;
}

static inline bool cpu_has_vmx_pml_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_ENABLE_PML;
}

static inline bool cpu_has_vmx_preemption_timer_bs(void)
{
	return vmcs_config_bs.pin_based_exec_ctrl &
		PIN_BASED_VMX_PREEMPTION_TIMER;
}

static inline bool cpu_has_vmx_posted_intr_bs(void)
{
	return IS_ENABLED(CONFIG_X86_LOCAL_APIC) &&
		vmcs_config_bs.pin_based_exec_ctrl & PIN_BASED_POSTED_INTR;
}

static inline bool cpu_has_vmx_tsc_scaling_bs(void)
{
	return vmcs_config_bs.cpu_based_2nd_exec_ctrl &
		SECONDARY_EXEC_TSC_SCALING;
}

static inline bool cpu_has_vmx_apicv_bs(void)
{
	return cpu_has_vmx_apic_register_virt_bs() &&
		cpu_has_vmx_virtual_intr_delivery_bs() &&
		cpu_has_vmx_posted_intr_bs();
}

static inline bool cpu_has_vmx_flexpriority_bs(void)
{
	return cpu_has_vmx_tpr_shadow_bs() &&
		cpu_has_vmx_virtualize_apic_accesses_bs();
}

static inline bool cpu_has_vmx_invvpid_bs(void)
{
	return vmx_capability_bs.vpid & VMX_VPID_INVVPID_BIT;
}

static inline bool cpu_has_vmx_invvpid_single_bs(void)
{
	return vmx_capability_bs.vpid & VMX_VPID_EXTENT_SINGLE_CONTEXT_BIT;
}

static inline bool cpu_has_vmx_invvpid_global_bs(void)
{
	return vmx_capability_bs.vpid & VMX_VPID_EXTENT_GLOBAL_CONTEXT_BIT;
}

static inline bool cpu_has_vmx_ept_4levels_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPT_PAGE_WALK_4_BIT;
}

static inline bool cpu_has_vmx_ept_mt_wb_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPTP_WB_BIT;
}

static inline bool cpu_has_vmx_invept_global_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPT_EXTENT_GLOBAL_BIT;
}

static inline bool cpu_has_vmx_ept_ad_bits_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPT_AD_BIT;
}

static inline bool cpu_has_vmx_ept_2m_page_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPT_2MB_PAGE_BIT;
}

static inline bool cpu_has_vmx_ept_execute_only_bs(void)
{
	return vmx_capability_bs.ept & VMX_EPT_EXECUTE_ONLY_BIT;
}

static inline bool cpu_has_vmx_intel_pt_bs(void)
{
	u64 vmx_msr;

	rdmsrl(MSR_IA32_VMX_MISC, vmx_msr);
	return (vmx_msr & MSR_IA32_VMX_MISC_INTEL_PT) &&
		(vmcs_config_bs.cpu_based_2nd_exec_ctrl & 
		SECONDARY_EXEC_PT_USE_GPA) &&
		(vmcs_config_bs.vmexit_ctrl & VM_EXIT_CLEAR_IA32_RTIT_CTL) &&
		(vmcs_config_bs.vmentry_ctrl & VM_ENTRY_LOAD_IA32_RTIT_CTL);
}
#endif /* __KVM_X86_VMX_CAPS_H */
