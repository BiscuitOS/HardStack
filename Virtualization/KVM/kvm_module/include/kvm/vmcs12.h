/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BISCUITOS_VMX_VMCS12_H
#define _BISCUITOS_VMX_VMCS12_H

#include <linux/build_bug.h>

#include "kvm/vmcs.h"

/*
 * struct vmcs12 describes the state that our guest hypervisor (L1) keeps for a
 * single nested guest (L2), hence the name vmcs12. Any VMX implementation has
 * a VMCS structure, and vmcs12 is our emulated VMX's VMCS. This structure is
 * stored in guest memory specified by VMPTRLD, but is opaque to the guest,
 * which must access it using VMREAD/VMWRITE/VMCLEAR instructions.
 * More than one of these structures may exist, if L1 runs multiple L2 guests.
 * nested_vmx_run() will use the data here to build the vmcs02: a VMCS for the
 * underlying hardware which will be used to run L2.
 * This structure is packed to ensure that its layout is identical across
 * machines (necessary for live migration).
 *
 * IMPORTANT: Changing the layout of existing fields in this structure
 * will break save/restore compatibility with older kvm releases. When
 * adding new fields, either use space in the reserved padding* arrays
 * or add the new fields to the end of the structure.
 */
typedef u64 natural_width;
struct __packed vmcs12 {
	/* According to the Intel spec, a VMCS region must start with the
	 * following two fields. Then follow implementation-specific data.
	 */
	struct vmcs_hdr hdr;
	u32 abort;

	u32 launch_state; /* set to 0 by VMCLEAR, to 1 by VMLAUNCH */
	u32 padding[7]; /* room for future expansion */

	u64 io_bitmap_a;
	u64 io_bitmap_b;
	u64 msr_bitmap;
	u64 vm_exit_msr_store_addr;
	u64 vm_exit_msr_load_addr;
	u64 vm_entry_msr_load_addr;
	u64 tsc_offset;
	u64 virtual_apic_page_addr;
	u64 apic_access_addr;
	u64 posted_intr_desc_addr;
	u64 ept_pointer;
	u64 eoi_exit_bitmap0;
	u64 eoi_exit_bitmap1;
	u64 eoi_exit_bitmap2;
	u64 eoi_exit_bitmap3;
	u64 xss_exit_bitmap;
	u64 guest_physical_address;
	u64 vmcs_link_pointer;
	u64 guest_ia32_debugctl;
	u64 guest_ia32_pat;
	u64 guest_ia32_efer;
	u64 guest_ia32_perf_global_ctrl;
	u64 guest_pdptr0;
	u64 guest_pdptr1;
	u64 guest_pdptr2;
	u64 guest_pdptr3;
	u64 guest_bndcfgs;
	u64 host_ia32_pat;
	u64 host_ia32_efer;
	u64 host_ia32_perf_global_ctrl;
	u64 vmread_bitmap;
	u64 vmwrite_bitmap;
	u64 vm_function_control;
	u64 eptp_list_address;
	u64 pml_address;
	u64 padding64[3]; /* room for future expansion */
	/*
	 * To allow migration of L1 (complete with its L2 guests) between
	 * machines of different natural widths (32 or 64 bit), we cannot have
	 * unsigned long fields with no explicit size. We use u64 (aliased
	 * natural_width) instead. Luckily, x86 is little-endian.
	 */
	natural_width cr0_guest_host_mask;
	natural_width cr4_guest_host_mask;
	natural_width cr0_read_shadow;
	natural_width cr4_read_shadow;
	natural_width cr3_target_value0;
	natural_width cr3_target_value1;
	natural_width cr3_target_value2;
	natural_width cr3_target_value3;
	natural_width exit_qualification;
	natural_width guest_linear_address;
	natural_width guest_cr0;
	natural_width guest_cr3;
	natural_width guest_cr4;
	natural_width guest_es_base;
	natural_width guest_cs_base;
	natural_width guest_ss_base;
	natural_width guest_ds_base;
	natural_width guest_fs_base;
	natural_width guest_gs_base;
	natural_width guest_ldtr_base;
	natural_width guest_tr_base;
	natural_width guest_gdtr_base;
	natural_width guest_idtr_base;
	natural_width guest_dr7;
	natural_width guest_rsp;
	natural_width guest_rip;
	natural_width guest_rflags;
	natural_width guest_pending_dbg_exceptions;
	natural_width guest_sysenter_esp;
	natural_width guest_sysenter_eip;
	natural_width host_cr0;
	natural_width host_cr3;
	natural_width host_cr4;
	natural_width host_fs_base;
	natural_width host_gs_base;
	natural_width host_tr_base;
	natural_width host_gdtr_base;
	natural_width host_idtr_base;
	natural_width host_ia32_sysenter_esp;
	natural_width host_ia32_sysenter_eip;
	natural_width host_rsp;
	natural_width host_rip;
	natural_width paddingl[8]; /* room for future expansion */
	u32 pin_based_vm_exec_control;
	u32 cpu_based_vm_exec_control;
	u32 exception_bitmap;
	u32 page_fault_error_code_mask;
	u32 page_fault_error_code_match;
	u32 cr3_target_count;
	u32 vm_exit_controls;
	u32 vm_exit_msr_store_count;
	u32 vm_exit_msr_load_count;
	u32 vm_entry_controls;
	u32 vm_entry_msr_load_count;
	u32 vm_entry_intr_info_field;
	u32 vm_entry_exception_error_code;
	u32 vm_entry_instruction_len;
	u32 tpr_threshold;
	u32 secondary_vm_exec_control;
	u32 vm_instruction_error;
	u32 vm_exit_reason;
	u32 vm_exit_intr_info;
	u32 vm_exit_intr_error_code;
	u32 idt_vectoring_info_field;
	u32 idt_vectoring_error_code;
	u32 vm_exit_instruction_len;
	u32 vmx_instruction_info;
	u32 guest_es_limit;
	u32 guest_cs_limit;
	u32 guest_ss_limit;
	u32 guest_ds_limit;
	u32 guest_fs_limit;
	u32 guest_gs_limit;
	u32 guest_ldtr_limit;
	u32 guest_tr_limit;
	u32 guest_gdtr_limit;
	u32 guest_idtr_limit;
	u32 guest_es_ar_bytes;
	u32 guest_cs_ar_bytes;
	u32 guest_ss_ar_bytes;
	u32 guest_ds_ar_bytes;
	u32 guest_fs_ar_bytes;
	u32 guest_gs_ar_bytes;
	u32 guest_ldtr_ar_bytes;
	u32 guest_tr_ar_bytes;
	u32 guest_interruptibility_info;
	u32 guest_activity_state;
	u32 guest_sysenter_cs;
	u32 host_ia32_sysenter_cs;
	u32 vmx_preemption_timer_value;
	u32 padding32[7]; /* room for future expansion */
	u16 virtual_processor_id;
	u16 posted_intr_nv;
	u16 guest_es_selector;
	u16 guest_cs_selector;
	u16 guest_ss_selector;
	u16 guest_ds_selector;
	u16 guest_fs_selector;
	u16 guest_gs_selector;
	u16 guest_ldtr_selector;
	u16 guest_tr_selector;
	u16 guest_intr_status;
	u16 host_es_selector;
	u16 host_cs_selector;
	u16 host_ss_selector;
	u16 host_ds_selector;
	u16 host_fs_selector;
	u16 host_gs_selector;
	u16 host_tr_selector;
	u16 guest_pml_index;
};

/*
 * VMCS12_REVISION is an arbitrary id that should be changed if the content or
 * layout of struct vmcs12 is changed. MSR_IA32_VMX_BASIC returns this id, and
 * VMPTRLD verifies that the VMCS region that L1 is loading contains this id.
 *
 * IMPORTANT: Changing this value will break save/restore compatibility with
 * older kvm releases.
 */
#define VMCS12_REVISION 0x11e57ed0

/*
 * VMCS12_SIZE is the number of bytes L1 should allocate for the VMXON region
 * and any VMCS region. Although only sizeof(struct vmcs12) are used by the
 * current implementation, 4K are reserved to avoid future complications.
 */
#define VMCS12_SIZE 0x1000

/*
 * VMCS12_MAX_FIELD_INDEX is the highest index value used in any
 * supported VMCS12 field encoding.
 */
#define VMCS12_MAX_FIELD_INDEX 0x17

/*
 * For save/restore compatibility, the vmcs12 field offsets must not change.
 */
#define CHECK_OFFSET(field, loc)					\
	BUILD_BUG_ON_MSG(offsetof(struct vmcs12, field) != (loc),	\
		"Offset of " #field " in struct vmcs12 has changed.")

static inline void vmx_check_vmcs12_offsets_bs(void)
{
	CHECK_OFFSET(hdr, 0);
}

#endif /* __KVM_X86_VMX_VMCS12_H */
