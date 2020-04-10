/*
 * X86 KVM Hyperv
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/kvm_host.h>

void kvm_hv_init_vm_bs(struct kvm *kvm)
{
	mutex_init(&kvm->arch.hyperv.hv_lock);
	idr_init(&kvm->arch.hyperv.conn_to_evt);
}
