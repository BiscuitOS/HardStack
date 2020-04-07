#ifndef _BISCUITOS_KVM_H
#define _BISCUITOS_KVM_H

#include "kvm/capabilities.h"

int kvm_init_bs(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
                                struct module *module);
int kvm_arch_init_bs(void *opaque);
int kvm_mmu_module_init_bs(void);
void kvm_mmu_set_mmio_spte_mask_bs(u64 mmio_mask, u64 mmio_value);
void kvm_mmu_set_mask_ptes_bs(u64 user_mask, u64 accessed_mask,
	u64 dirty_mask, u64 nx_mask, u64 x_mask, u64 p_mask,
	u64 acc_track_mask, u64 me_mask);
void kvm_lapic_init_bs(void);
int kvm_irqfd_init_bs(void);
int kvm_arch_hardware_setup_bs(void);
void kvm_define_shared_msr_bs(unsigned slot, u32 msr);
void evmcs_sanitize_exec_ctrls_bs(struct vmcs_config *vmcs_conf);
extern struct static_key_false enable_evmcs_bs;

#define BS_DUP()	printk("Expand..[%s-%d]\n", __func__, __LINE__)

#endif
