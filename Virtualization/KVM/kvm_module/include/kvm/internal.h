#ifndef _BISCUITOS_KVM_H
#define _BISCUITOS_KVM_H

#include <linux/kvm_host.h>
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
void kvm_enable_efer_bits_bs(u64 mask);
void kvm_disable_largepages_bs(void);
void kvm_enable_tdp_bs(void);
void kvm_disable_tdp_bs(void);
void __kvm_request_immediate_exit_bs(struct kvm_vcpu *vcpu);
void kvm_arch_check_processor_compat_bs(void *rtn);
void nested_vmx_setup_ctls_msrs_bs(struct nested_vmx_msrs *msrs,
                                u32 ept_caps, bool apicv);
__init int 
nested_vmx_hardware_setup_bs(int (*exit_handlers[])(struct kvm_vcpu *));
int nested_enable_evmcs_bs(struct kvm_vcpu *vcpu, uint16_t *vmcs_version);
uint16_t nested_get_evmcs_version_bs(struct kvm_vcpu *vcpu);
bool kvm_mpx_supported_bs(void);
int vmx_get_vmx_msr_bs(struct nested_vmx_msrs *, u32, u64 *);
int kvm_async_pf_init_bs(void);
static const struct file_operations *stat_fops_bs[];
int kvm_register_device_ops_bs(struct kvm_device_ops *ops, u32 type);
int kvm_vfio_ops_init_bs(void);

extern struct kvm_x86_ops *kvm_x86_ops_bs;
extern struct static_key_false enable_evmcs_bs;
extern bool    __read_mostly kvm_has_tsc_control_bs;
extern u64     __read_mostly kvm_max_tsc_scaling_ratio_bs;
extern u8      __read_mostly kvm_tsc_scaling_ratio_frac_bits_bs;
extern bool tdp_enabled_bs;
extern bool enable_ept_bs;
extern bool enable_ept_ad_bits_bs;
extern bool enable_vpid_bs;
extern bool enable_unrestricted_guest_bs;
extern bool flexpriority_enabled_bs;
extern u64 host_xcr0_bs;
extern struct kvm_stats_debugfs_item debugfs_entries_bs[];

#define BS_DUP()	printk("Expand..[%s-%d]\n", __func__, __LINE__)

#endif
