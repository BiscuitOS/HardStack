/*
 * MSR_IA32_VMX_PINBASED_CTLS [X86]
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/msr.h>

/*
 * IA32_VMX_PINBASED_CTLS - Capability Reporting Register of Pinbased
 *                          VM-execution Controls (R/O)
 *
 * The IA32_VMX_PINBASED_CTLS MSR (index 481H) reports on the allowed settings
 * of most of the pin-based VMX-execution controls.
 *
 * • Bits 31:0 indicate the allowed 0-settings of these controls. VM entry
 *   allows control X (bit X of the pin-based VM-exection control) to be 0
 *   if bit X in the MSR is cleared to 0; if bit X in the MSR is set to 1,
 *   VM entry fails if control X is 0.
 *   More information on A.3.1
 *
 * • Bits 63:32 indicate the allowed 1-settings of these controls. VM entry
 *   allows control X to be 1 if bit 32+X in the MSR is set to 1; if bit 32
 *   +X in the MSR is cleared to 0, VM entry fails if control X is 1.
 *
 * 24.6.1 Pin-Based VM-Execution Control
 *  
 * The pin-based VM-execution controls constitute a 32-bit vector that governs
 * the handing of asynchronous events (for example: interrupts). Table list the
 * controls.
 *
 * ====+====================+================================================
 * Bit | NAame              | Description
 * ----+--------------------+------------------------------------------------
 * 0   | External-interrupt | If this control is 1, external interrupt casuse
 *     | exiting            | VM exits. Otherwise, they are delivered normally
 *     |                    | through the guest interrupt-descriptor table 
 *     |                    | (IDT). If this control is 1, the value of 
 *     |                    | RFLAGS.IF does not affect interrupt blocking.
 * ----+--------------------+------------------------------------------------
 * 3   | NMI exiting        | If this control is 1, non-maskable interrupt
 *     |                    | (NMIs) cause VM exits. Othersize, thet are deliv
 *     |                    | -ered normally using descriptor 2 of the IDT.
 *     |                    | This control also determines interactions between
 *     |                    | IRET and blocking by NMI.
 * ----+--------------------+------------------------------------------------
 * 5   | Virtual NMIs       | If this control 1, NMIs are never blocked and the
 *     |                    | "blocking by NMI" bit (bit 3) in the interruptib
 *     |                    | -ility-state field indicates "virtual-NMI block
 *     |                    | -ing". This control also interacts with the "NMI
 *     |                    | -window exiting" VM-executing control.
 * ----+--------------------+------------------------------------------------
 * 6   | Activate VMX-      | If this control is 1, the VMX-preemption timer
 *     | preemption timer   | counts down in VMX non-root operation. A VM exit
 *     |                    | occurs when the timer counts down to zero.
 * ----+--------------------+------------------------------------------------
 * 7   | Process posted     | If this control is 1, the processor treats inter
 *     | interrupts         | -rupts with the posted-interrupt notification 
 *     |                    | vector specially, updating the virtual-APIC page
 *     |                    | with posted-interrupt requests.
 * ----+--------------------+------------------------------------------------
 *
 * All other bits in this field are reserved, some to 0 and some to 1. Software
 * should consult the VMX capability MSRs IA32_VMX_PINBASED_CTLS and IA32_VMX_-
 * TRUE_PINBASED_CTLS to determine how to set reserved bit. Failure to set
 * reserved bits properly causes subsequent VM entries to fail.
 *
 * The first processors to support the virtual-machine extensions supported 
 * only the 1-settings of bits 1,2, and 4. The VMX capability MSR IA32_VMX_-
 * PINBASED_CTLS will always report that these bit must be 1. Logical process
 * that support the 0-settting of any of these bits will support the VMX capa-
 * bility MSR.
 * IA32_VMX_TRUE_PINBASED_CTLS MSR, and software should consult this MSR to 
 * discover support for the 0-settings of these bits. Software that is not
 * aware of the functionality of any one of these bits should set that bit to 1.
 */

#define MSR_IA32_VMX_PINBASED_CTLS	0x00000481 
#define PIN_BASED_EXT_INTR_MASK		0x00000001
#define PIN_BASED_NMI_EXITING		0x00000008
#define PIN_BASED_VIRTUAL_NMIS		0x00000020

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	u32 msr_low, msr_high;
	u32 ctl_min, ctl_opt;
	u32 ctl;

	ctl_min = PIN_BASED_EXT_INTR_MASK | PIN_BASED_NMI_EXITING;
	ctl_opt = PIN_BASED_VIRTUAL_NMIS;
	ctl = ctl_min | ctl_opt;

	rdmsr(MSR_IA32_VMX_PINBASED_CTLS, msr_low, msr_high);

	printk("MSR %#x:%#x\n", msr_high, msr_low);
	printk("Default ctl_min: %#x ctl_opt: %#x ctl: %#x\n", 
						ctl_min, ctl_opt, ctl);

	/* bit == 0 in high word ==> must be zero */
	ctl &= msr_high;
	printk("High 1-settings: %#x\n", ctl);
	/* bit == 1 in low word ==> must be one */
	ctl |= msr_low;
	printk("Lows 0-settings: %#x\n", ctl);

	printk("CTL_min %#x and ~CTL %#x\n", ctl_min, ~ctl);
	/* information */
	if (ctl_min & ~ctl)
		printk("ctl_min not required.\n");
	else
		printk("The required CTL %#x.\n", ctl);

	printk("Hello modules on BiscuitOS\n");

	return 0;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
