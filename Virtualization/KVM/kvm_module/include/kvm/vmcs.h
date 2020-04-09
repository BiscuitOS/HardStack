/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KVM_X86_VMX_VMCS_H
#define __KVM_X86_VMX_VMCS_H

#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/nospec.h>

#include <asm/kvm.h>
#include <asm/vmx.h>

#include "kvm/capabilities.h"

struct vmcs_hdr {
	u32 revision_id:31;
	u32 shadow_vmcs:1;
};

struct vmcs {
	struct vmcs_hdr hdr;
	u32 abort;
	char data[0];
};

DECLARE_PER_CPU(struct vmcs *, current_vmcs);

/*
 * vmcs_host_state tracks registers that are loaded from the VMCS on VMEXIT
 * and whose values change infrequently, but are not constant.  I.e. this is
 * used as a write-through cache of the corresponding VMCS fields.
 */
struct vmcs_host_state {
	unsigned long cr3;	/* May not match real cr3 */
	unsigned long cr4;	/* May not match real cr4 */
	unsigned long gs_base;
	unsigned long fs_base;

	u16           fs_sel, gs_sel, ldt_sel;
#ifdef CONFIG_X86_64
	u16           ds_sel, es_sel;
#endif
};

/*
 * Track a VMCS that may be loaded on a certain CPU. If it is (cpu!=-1), also
 * remember whether it was VMLAUNCHed, and maintain a linked list of all VMCSs
 * loaded on this CPU (so we can clear them if the CPU goes down).
 */
struct loaded_vmcs {
	struct vmcs *vmcs;
	struct vmcs *shadow_vmcs;
	int cpu;
	bool launched;
	bool nmi_known_unmasked;
	bool hv_timer_armed;
	/* Support for vnmi-less CPUs */
	int soft_vnmi_blocked;
	ktime_t entry_time;
	s64 vnmi_blocked_time;
	unsigned long *msr_bitmap;
	struct list_head loaded_vmcss_on_cpu_link;
	struct vmcs_host_state host_state;
};

enum vmcs_field_width {
	VMCS_FIELD_WIDTH_U16 = 0,
	VMCS_FIELD_WIDTH_U64 = 1,
	VMCS_FIELD_WIDTH_U32 = 2,
	VMCS_FIELD_WIDTH_NATURAL_WIDTH = 3
};

static inline int vmcs_field_width_bs(unsigned long field)
{
	if (0x1 & field)
		return VMCS_FIELD_WIDTH_U32;
	return (field >> 13) & 0x3;
}

#endif /* __KVM_X86_VMX_VMCS_H */
