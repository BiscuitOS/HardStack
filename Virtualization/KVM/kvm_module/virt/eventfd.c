/*
 * X86 KVM eventfd
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/kvm_host.h>

static struct workqueue_struct *irqfd_cleanup_wq_bs;

/*
 * create a host-wide workqueue for issuing deferred shutdown requrests
 * aggregated from all vm* instances. We need our own isoloated queue
 * to ease flushing work items when a VM exits.
 */
int kvm_irqfd_init_bs(void)
{
	irqfd_cleanup_wq_bs = alloc_workqueue("kvm-irqfd-cleanup-bs", 0, 0);
	if (!irqfd_cleanup_wq_bs)
		return -ENOMEM;

	return 0;
}

void kvm_eventfd_init_bs(struct kvm *kvm)
{
#ifdef CONFIG_HAVE_KVM_IRQFD
	spin_lock_init(&kvm->irqfds.lock);
	INIT_LIST_HEAD(&kvm->irqfds.items);
	INIT_LIST_HEAD(&kvm->irqfds.resampler_list);
	mutex_init(&kvm->irqfds.resampler_lock);
#endif
	INIT_LIST_HEAD(&kvm->ioeventfds);
}
