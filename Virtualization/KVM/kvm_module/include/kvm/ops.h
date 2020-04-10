#ifndef _BISCUITOS_KVM_OPS_H
#define _BISCUITOS_KVM_OPS_H

#include <linux/mm.h>
#include <asm/kvm_host.h>

/*
 * Hardware virtualization extension instructions may fault if a
 * reboot turns off virtualization while processes are running.
 * Trap the fault and ignore the instruction if that happens.
 */
asmlinkage void kvm_spurious_fault_bs(void);

extern bool kvm_rebooting_bs;

#define ____kvm_handle_fault_on_reboot_bs(insn, cleanup_insn)      \
	"666: " insn "\n\t" \
	"668: \n\t"                           \
	".pushsection .fixup, \"ax\" \n" \
	"667: \n\t" \
	cleanup_insn "\n\t"                   \
	"cmpb $0, kvm_rebooting_bs \n\t"         \
	"jne 668b \n\t"                       \
	__ASM_SIZE(push) " $666b \n\t"        \
	"jmp kvm_spurious_fault_bs \n\t"         \
	".popsection \n\t" \
	_ASM_EXTABLE(666b, 667b)

#define __kvm_handle_fault_on_reboot_bs(insn)              \
	____kvm_handle_fault_on_reboot_bs(insn, "")

#define __ex_bs(x)	__kvm_handle_fault_on_reboot_bs(x)

static inline void vmcs_clear_bs(struct vmcs *vmcs)
{
	u64 phys_addr = __pa(vmcs);
	bool error;

	asm volatile (__ex_bs("vmclear %1") CC_SET(na)
			: CC_OUT(na) (error) : "m"(phys_addr));
	if (unlikely(error))
		printk(KERN_ERR "kvm: vmclear fail: %p/%llx\n",
				vmcs, phys_addr);
}

static inline void __invept_bs(unsigned long ext, u64 eptp, gpa_t gpa)
{
	struct {
		u64 eptp, gpa;
	} operand = { eptp, gpa };
	bool error;

	asm volatile (__ex_bs("invept %2, %1") CC_SET(na)
			: CC_OUT(na) (error) : "r" (ext), "m" (operand));
	BUG_ON(error);
}

static inline void ept_sync_global_bs(void)
{
	__invept_bs(VMX_EPT_EXTENT_GLOBAL, 0, 0);
}

#endif
