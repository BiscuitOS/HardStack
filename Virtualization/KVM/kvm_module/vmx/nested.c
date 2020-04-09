/*
 * KVM: nested
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/types.h>
#include <linux/module.h>
#include <asm/msr-index.h>
#include <asm/vmx.h>
#include <asm/msr.h>

#include "kvm/capabilities.h"
#include "kvm/vmcs12.h"
#include "kvm/internal.h"
#include "kvm/vmcs.h"

static bool __read_mostly enable_shadow_vmcs_bs = 1;
module_param_named(enable_shadow_vmcs, enable_shadow_vmcs_bs, bool, S_IRUGO);

static bool __read_mostly nested_early_check_bs = 0;
module_param(nested_early_check_bs, bool, S_IRUGO);

/*
 * Hyper-V requires all of these, so mark them as supported even though
 * they are just treated the same as all-context.
 */
#define VMX_VPID_EXTENT_SUPPORTED_MASK          \
	(VMX_VPID_EXTENT_INDIVIDUAL_ADDR_BIT |  \
	VMX_VPID_EXTENT_SINGLE_CONTEXT_BIT |    \
	VMX_VPID_EXTENT_GLOBAL_CONTEXT_BIT |    \
	VMX_VPID_EXTENT_SINGLE_NON_GLOBAL_BIT)

#define VMX_MISC_EMULATED_PREEMPTION_TIMER_RATE	5

enum {
	VMX_VMREAD_BITMAP,
	VMX_VMWRITE_BITMAP,
	VMX_BITMAP_NR
};
static unsigned long *vmx_bitmap_bs[VMX_BITMAP_NR];

#define vmx_vmread_bitmap_bs		(vmx_bitmap_bs[VMX_VMREAD_BITMAP])
#define vmx_vmwrite_bitmap_bs		(vmx_bitmap_bs[VMX_VMWRITE_BITMAP])

static u16 shadow_read_only_fields_bs[] = {
#define SHADOW_FIELD_RO(x) x,
#include "kvm/vmcs_shadow_fields.h"
};
static int max_shadow_read_only_fields_bs =
		ARRAY_SIZE(shadow_read_only_fields_bs);

static u16 shadow_read_write_fields_bs[] = {
#define SHADOW_FIELD_RW(x) x,
#include "kvm/vmcs_shadow_fields.h"
};
static int max_shadow_read_write_fields_bs =
		ARRAY_SIZE(shadow_read_write_fields_bs);

static void init_vmcs_shadow_fields_bs(void)
{
	int i, j;

	memset(vmx_vmread_bitmap_bs, 0xff, PAGE_SIZE);
	memset(vmx_vmwrite_bitmap_bs, 0xff, PAGE_SIZE);

	for (i = j = 0; i < max_shadow_read_only_fields_bs; i++) {
		u16 field = shadow_read_only_fields_bs[i];

		if (vmcs_field_width_bs(field) == VMCS_FIELD_WIDTH_U64 &&
			(i + 1 == max_shadow_read_only_fields_bs ||
			shadow_read_only_fields_bs[i + 1] != field + 1))
			pr_err("Missing field from shadow_read_only_field "
							"%x\n", field + 1);

		clear_bit(field, vmx_vmread_bitmap_bs);
		if (j < i)
			shadow_read_only_fields_bs[j] = field;
		j++;
	}
	max_shadow_read_only_fields_bs = j;

	for (i = j = 0; i < max_shadow_read_write_fields_bs; i++) {
		u16 field = shadow_read_write_fields_bs[i];

		if (vmcs_field_width_bs(field) == VMCS_FIELD_WIDTH_U64 &&
			(i + 1 == max_shadow_read_write_fields_bs ||
			shadow_read_write_fields_bs[i + 1] != field + 1))
			pr_err("Missing field from shadow_read_write_field "
						"%x\n", field + 1);

		/*
		 * PML and the preemption timer can be emulated, but the
		 * processor cannot vmwrite to fields that don't exist
		 * on bare metal.
		 */
		switch (field) {
		case GUEST_PML_INDEX:
			if (!cpu_has_vmx_pml_bs())
				continue;
			break;
		case VMX_PREEMPTION_TIMER_VALUE:
			if (!cpu_has_vmx_preemption_timer_bs())
				continue;
			break;
		case GUEST_INTR_STATUS:
			if (!cpu_has_vmx_apicv_bs())
				continue;
			break;
		default:
			break;
		}

		clear_bit(field, vmx_vmwrite_bitmap_bs);
		clear_bit(field, vmx_vmread_bitmap_bs);
		if (j < i)
			shadow_read_write_fields_bs[j] = field;
		j++;
	}
	max_shadow_read_write_fields_bs = j;
}


static int handle_vmclear_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmlaunch_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmptrld_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmptrst_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmread_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmresume_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmwrite_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmoff_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmon_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invept_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_invvpid_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int handle_vmfunc_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
	return 0;
}

static int 
vmx_check_nested_events_bs(struct kvm_vcpu *vcpu, bool external_inter)
{
	BS_DUP();
	return 0;
}

static int vmx_get_nested_state_bs(struct kvm_vcpu *vcpu,
			struct kvm_nested_state __user *user_kvm_nested_state,
			u32 user_data_size)
{
	BS_DUP();
	return 0;
}

static int vmx_set_nested_state_bs(struct kvm_vcpu *vcpu,
			struct kvm_nested_state __user *user_kvm_nested_state,
			struct kvm_nested_state *kvm_state)
{
	BS_DUP();
	return 0;
}

static void nested_get_vmcs12_pages_bs(struct kvm_vcpu *vcpu)
{
	BS_DUP();
}

/*
 * nested_vmx_setup_ctls_msrs() sets up variables containing the values to be
 * returned for the various VMX controls MSRs when nested VMX is enabled.
 * The same values should also be used to verify that vmcs12 control fields are
 * valid during nested entry from L1 to L2.
 * Each of these control msrs has a low and high 32-bit half: A low bit is on
 * if the corresponding bit in the (32-bit) control field *must* be on, and a
 * bit in the high half is on if the corresponding bit in the control field
 * may be on. See also vmx_control_verify().
 */
void nested_vmx_setup_ctls_msrs_bs(struct nested_vmx_msrs *msrs,
				u32 ept_caps, bool apicv)
{
	/*
	 * Note that as a general rule, the high half of the MSRs (bits in
	 * the control fields which may be 1) should be initialized by the
	 * intersection of the underlying hardware's MSR (i.e., features which
	 * can be supported) and the list of features we want to expose -
	 * because they are known to be properly supported in our code.
	 * Also, usually, the low half of the MSRs (bits which must be 1) can
	 * be set to 0, meaning that L1 may turn off any of these bits. The
	 * reason is that if one of these bits is necessary, it will appear
	 * in vmcs01 and prepare_vmcs02, when it bitwise-or's the control
	 * fields of vmcs01 and vmcs02, will turn these bits off - and
	 * nested_vmx_exit_reflected() will not pass related exits to L1.
	 * These rules have exceptions below.
	 */

	/* pin-based controls */
	rdmsr(MSR_IA32_VMX_PINBASED_CTLS,
		msrs->pinbased_ctls_low,
		msrs->pinbased_ctls_high);
	msrs->pinbased_ctls_low |=
		PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
	msrs->pinbased_ctls_high &=
		PIN_BASED_EXT_INTR_MASK |
		PIN_BASED_NMI_EXITING |
		PIN_BASED_VIRTUAL_NMIS |
		(apicv ? PIN_BASED_POSTED_INTR : 0);
	msrs->pinbased_ctls_high |=
		PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR |
		PIN_BASED_VMX_PREEMPTION_TIMER;


	/* exit controls */
	rdmsr(MSR_IA32_VMX_EXIT_CTLS,
	msrs->exit_ctls_low,
		msrs->exit_ctls_high);
	msrs->exit_ctls_low =
		VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR;

	msrs->exit_ctls_high &=
		VM_EXIT_LOAD_IA32_PAT | VM_EXIT_SAVE_IA32_PAT;
	msrs->exit_ctls_high |=
		VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR |
		VM_EXIT_LOAD_IA32_EFER | VM_EXIT_SAVE_IA32_EFER |
		VM_EXIT_SAVE_VMX_PREEMPTION_TIMER | VM_EXIT_ACK_INTR_ON_EXIT;

	/* We support free control of debug control saving. */
	msrs->exit_ctls_low &= ~VM_EXIT_SAVE_DEBUG_CONTROLS;

	/* entry controls */
	rdmsr(MSR_IA32_VMX_ENTRY_CTLS,
	msrs->entry_ctls_low,
		msrs->entry_ctls_high);
	msrs->entry_ctls_low =
		VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR;
	msrs->entry_ctls_high &=
		VM_ENTRY_LOAD_IA32_PAT;
	msrs->entry_ctls_high |=
		(VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR | VM_ENTRY_LOAD_IA32_EFER);

	/* We support free control of debug control loading. */
	msrs->entry_ctls_low &= ~VM_ENTRY_LOAD_DEBUG_CONTROLS;

	/* cpu-based controls */
	rdmsr(MSR_IA32_VMX_PROCBASED_CTLS,
	msrs->procbased_ctls_low,
		msrs->procbased_ctls_high);
	msrs->procbased_ctls_low =
		CPU_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
	msrs->procbased_ctls_high &=
		CPU_BASED_VIRTUAL_INTR_PENDING |
		CPU_BASED_VIRTUAL_NMI_PENDING | CPU_BASED_USE_TSC_OFFSETING |
		CPU_BASED_HLT_EXITING | CPU_BASED_INVLPG_EXITING |
		CPU_BASED_MWAIT_EXITING | CPU_BASED_CR3_LOAD_EXITING |
		CPU_BASED_CR3_STORE_EXITING |
		CPU_BASED_MOV_DR_EXITING | CPU_BASED_UNCOND_IO_EXITING |
		CPU_BASED_USE_IO_BITMAPS | CPU_BASED_MONITOR_TRAP_FLAG |
		CPU_BASED_MONITOR_EXITING | CPU_BASED_RDPMC_EXITING |
		CPU_BASED_RDTSC_EXITING | CPU_BASED_PAUSE_EXITING |
		CPU_BASED_TPR_SHADOW | CPU_BASED_ACTIVATE_SECONDARY_CONTROLS;

	/*
	 * We can allow some features even when not supported by the
	 * hardware. For example, L1 can specify an MSR bitmap - and we
	 * can use it to avoid exits to L1 - even when L0 runs L2
	 * without MSR bitmaps.
	 */
	msrs->procbased_ctls_high |=
		CPU_BASED_ALWAYSON_WITHOUT_TRUE_MSR |
		CPU_BASED_USE_MSR_BITMAPS;

	/* We support free control of CR3 access interception. */
	msrs->procbased_ctls_low &=
		~(CPU_BASED_CR3_LOAD_EXITING | CPU_BASED_CR3_STORE_EXITING);

	/*
	 * secondary cpu-based controls.  Do not include those that
	 * depend on CPUID bits, they are added later by vmx_cpuid_update.
	 */
	if (msrs->procbased_ctls_high & CPU_BASED_ACTIVATE_SECONDARY_CONTROLS)
		rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2,
			msrs->secondary_ctls_low,
			msrs->secondary_ctls_high);

	msrs->secondary_ctls_low = 0;
	msrs->secondary_ctls_high &=
		SECONDARY_EXEC_DESC |
		SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE |
		SECONDARY_EXEC_APIC_REGISTER_VIRT |
		SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY |
		SECONDARY_EXEC_WBINVD_EXITING;

	/*
	 * We can emulate "VMCS shadowing," even if the hardware
	 * doesn't support it.
	 */
	msrs->secondary_ctls_high |=
		SECONDARY_EXEC_SHADOW_VMCS;

	if (enable_ept_bs) {
		/* nested EPT: emulate EPT also to L1 */
		msrs->secondary_ctls_high |=
			SECONDARY_EXEC_ENABLE_EPT;
		msrs->ept_caps = VMX_EPT_PAGE_WALK_4_BIT |
			VMX_EPTP_WB_BIT | VMX_EPT_INVEPT_BIT;
		if (cpu_has_vmx_ept_execute_only_bs())
			msrs->ept_caps |=
				VMX_EPT_EXECUTE_ONLY_BIT;
		msrs->ept_caps &= ept_caps;
		msrs->ept_caps |= VMX_EPT_EXTENT_GLOBAL_BIT |
				  VMX_EPT_EXTENT_CONTEXT_BIT | 
				  VMX_EPT_2MB_PAGE_BIT |
				  VMX_EPT_1GB_PAGE_BIT;
		if (enable_ept_ad_bits_bs) {
			msrs->secondary_ctls_high |=
				  SECONDARY_EXEC_ENABLE_PML;
			msrs->ept_caps |= VMX_EPT_AD_BIT;
		}
	}

	if (cpu_has_vmx_vmfunc_bs()) {
		msrs->secondary_ctls_high |=
			SECONDARY_EXEC_ENABLE_VMFUNC;
		/*
		 * Advertise EPTP switching unconditionally
		 * since we emulate it
		 */
		if (enable_ept_bs)
			msrs->vmfunc_controls =
				VMX_VMFUNC_EPTP_SWITCHING;
	}

	/*
	 * Old versions of KVM use the single-context version without
	 * checking for support, so declare that it is supported even
	 * though it is treated as global context.  The alternative is
	 * not failing the single-context invvpid, and it is worse.
	 */
	if (enable_vpid_bs) {
		msrs->secondary_ctls_high |=
			SECONDARY_EXEC_ENABLE_VPID;
		msrs->vpid_caps = VMX_VPID_INVVPID_BIT |
			VMX_VPID_EXTENT_SUPPORTED_MASK;
	}

	if (enable_unrestricted_guest_bs)
		msrs->secondary_ctls_high |=
			SECONDARY_EXEC_UNRESTRICTED_GUEST;

	if (flexpriority_enabled_bs)
		msrs->secondary_ctls_high |=
			SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES;

	/* miscellaneous data */
	rdmsr(MSR_IA32_VMX_MISC,
		msrs->misc_low,
		msrs->misc_high);
	msrs->misc_low &= VMX_MISC_SAVE_EFER_LMA;
	msrs->misc_low |=
		MSR_IA32_VMX_MISC_VMWRITE_SHADOW_RO_FIELDS |
		VMX_MISC_EMULATED_PREEMPTION_TIMER_RATE |
		VMX_MISC_ACTIVITY_HLT;
	msrs->misc_high = 0;

	/*
	 * This MSR reports some information about VMX support. We
	 * should return information about the VMX we emulate for the
	 * guest, and the VMCS structure we give it - not about the
	 * VMX support of the underlying hardware.
	 */
	msrs->basic =
		VMCS12_REVISION |
		VMX_BASIC_TRUE_CTLS |
		((u64)VMCS12_SIZE << VMX_BASIC_VMCS_SIZE_SHIFT) |
		(VMX_BASIC_MEM_TYPE_WB << VMX_BASIC_MEM_TYPE_SHIFT);

	if (cpu_has_vmx_basic_inout_bs())
		msrs->basic |= VMX_BASIC_INOUT;

	/*
	 * These MSRs specify bits which the guest must keep fixed on
	 * while L1 is in VMXON mode (in L1's root mode, or running an L2).
	 * We picked the standard core2 setting.
	 */
#define VMXON_CR0_ALWAYSON	(X86_CR0_PE | X86_CR0_PG | X86_CR0_NE)
#define VMXON_CR4_ALWAYSON	X86_CR4_VMXE
	msrs->cr0_fixed0 = VMXON_CR0_ALWAYSON;
	msrs->cr4_fixed0 = VMXON_CR4_ALWAYSON;

	/* These MSRs specify bits which the guest must keep fixed off. */
	rdmsrl(MSR_IA32_VMX_CR0_FIXED1, msrs->cr0_fixed1);
	rdmsrl(MSR_IA32_VMX_CR4_FIXED1, msrs->cr4_fixed1);

	/* highest index: VMX_PREEMPTION_TIMER_VALUE */
	msrs->vmcs_enum = VMCS12_MAX_FIELD_INDEX << 1;
}

__init int 
nested_vmx_hardware_setup_bs(int (*exit_handlers[])(struct kvm_vcpu *))
{                                          
	int i;
        
	if (!cpu_has_vmx_shadow_vmcs_bs())
		enable_shadow_vmcs_bs = 0;
	if (enable_shadow_vmcs_bs) {
		for (i = 0; i < VMX_BITMAP_NR; i++) {
			vmx_bitmap_bs[i] = 
				(unsigned long *)__get_free_page(GFP_KERNEL);
			if (!vmx_bitmap_bs[i]) {
				BS_DUP();
			}
		}

		init_vmcs_shadow_fields_bs();
	}

	exit_handlers[EXIT_REASON_VMCLEAR]	= handle_vmclear_bs,
	exit_handlers[EXIT_REASON_VMLAUNCH]	= handle_vmlaunch_bs,
	exit_handlers[EXIT_REASON_VMPTRLD]	= handle_vmptrld_bs,
	exit_handlers[EXIT_REASON_VMPTRST]	= handle_vmptrst_bs,
	exit_handlers[EXIT_REASON_VMREAD]	= handle_vmread_bs,
	exit_handlers[EXIT_REASON_VMRESUME]	= handle_vmresume_bs,
	exit_handlers[EXIT_REASON_VMWRITE]	= handle_vmwrite_bs,
	exit_handlers[EXIT_REASON_VMOFF]	= handle_vmoff_bs,
	exit_handlers[EXIT_REASON_VMON]		= handle_vmon_bs,
	exit_handlers[EXIT_REASON_INVEPT]	= handle_invept_bs,
	exit_handlers[EXIT_REASON_INVVPID]	= handle_invvpid_bs,
	exit_handlers[EXIT_REASON_VMFUNC]	= handle_vmfunc_bs,

	kvm_x86_ops_bs->check_nested_events = vmx_check_nested_events_bs;
	kvm_x86_ops_bs->get_nested_state = vmx_get_nested_state_bs;
	kvm_x86_ops_bs->set_nested_state = vmx_set_nested_state_bs;
	kvm_x86_ops_bs->get_vmcs12_pages = nested_get_vmcs12_pages_bs,
	kvm_x86_ops_bs->nested_enable_evmcs = nested_enable_evmcs_bs;
	kvm_x86_ops_bs->nested_get_evmcs_version = nested_get_evmcs_version_bs;

	return 0;
}

static inline u64 vmx_control_msr_bs(u32 low, u32 high)
{
	return low | ((u64)high << 32);
}

/* Returns 0 on success, non-0 otherwise. */
int 
vmx_get_vmx_msr_bs(struct nested_vmx_msrs *msrs, u32 msr_index, u64 *pdata)
{
	switch (msr_index) {
	case MSR_IA32_VMX_BASIC:
		*pdata = msrs->basic;
		break;
	case MSR_IA32_VMX_TRUE_PINBASED_CTLS:
	case MSR_IA32_VMX_PINBASED_CTLS:
		*pdata = vmx_control_msr_bs(
				msrs->pinbased_ctls_low,
				msrs->pinbased_ctls_high);
		if (msr_index == MSR_IA32_VMX_PINBASED_CTLS)
			*pdata |= PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_PROCBASED_CTLS:
	case MSR_IA32_VMX_PROCBASED_CTLS:
		*pdata = vmx_control_msr_bs(
				msrs->procbased_ctls_low,
				msrs->procbased_ctls_high);
		if (msr_index == MSR_IA32_VMX_PROCBASED_CTLS)
			*pdata |= CPU_BASED_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_EXIT_CTLS:
	case MSR_IA32_VMX_EXIT_CTLS:
		*pdata = vmx_control_msr_bs(
				msrs->exit_ctls_low,
				msrs->exit_ctls_high);
		if (msr_index == MSR_IA32_VMX_EXIT_CTLS)
			*pdata |= VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_TRUE_ENTRY_CTLS:
	case MSR_IA32_VMX_ENTRY_CTLS:
		*pdata = vmx_control_msr_bs(
				msrs->entry_ctls_low,
				msrs->entry_ctls_high);
		if (msr_index == MSR_IA32_VMX_ENTRY_CTLS)
			*pdata |= VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR;
		break;
	case MSR_IA32_VMX_MISC:
		*pdata = vmx_control_msr_bs(
				msrs->misc_low,
				msrs->misc_high);
		break;
	case MSR_IA32_VMX_CR0_FIXED0:
		*pdata = msrs->cr0_fixed0;
		break;
	case MSR_IA32_VMX_CR0_FIXED1:
		*pdata = msrs->cr0_fixed1;
		break;
	case MSR_IA32_VMX_CR4_FIXED0:
		*pdata = msrs->cr4_fixed0;
		break;
	case MSR_IA32_VMX_CR4_FIXED1:
		*pdata = msrs->cr4_fixed1;
		break;
	case MSR_IA32_VMX_VMCS_ENUM:
		*pdata = msrs->vmcs_enum;
		break;
	case MSR_IA32_VMX_PROCBASED_CTLS2:
		*pdata = vmx_control_msr_bs(
				msrs->secondary_ctls_low,
				msrs->secondary_ctls_high);
		break;
	case MSR_IA32_VMX_EPT_VPID_CAP:
		*pdata = msrs->ept_caps |
				((u64)msrs->vpid_caps << 32);
		break;
	case MSR_IA32_VMX_VMFUNC:
		*pdata = msrs->vmfunc_controls;
		break;
	default:
		return 1;
	}

	return 0;
}
