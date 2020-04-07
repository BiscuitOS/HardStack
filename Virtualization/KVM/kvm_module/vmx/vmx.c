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

#include "kvm/internal.h"
#include "kvm/vmx.h"

static int vmx_vm_init_bs(struct kvm *kvm)
{
	BS_DUP();
}

static struct kvm_x86_ops vmx_x86_ops_bs __ro_after_init = {
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
