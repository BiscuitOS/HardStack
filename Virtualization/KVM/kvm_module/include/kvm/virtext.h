#ifndef _BISCUITOS_VIRTEXT_H
#define _BISCUITOS_VIRTEXT_H
/* CPU virtualization extensions handling
 *
 * This should carry the code for handling CPU virtualization extensions
 * that needs to live in the kernel core.
 *
 * Author: Eduardo Habkost <ehabkost@redhat.com> 
 *
 * Copyright (C) 2008, Red Hat Inc.
 *      
 * Contains code from KVM, Copyright (C) 2006 Qumranet, Inc.
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

static inline int cpu_has_vmx_bs(void)
{
	unsigned long ecx = cpuid_ecx(1);
	return test_bit(5, &ecx); /* CPUID.1:ECX.VMX[bit 5] -> VT */
}

#endif
