/*
 * kvm_main.c
 *
 * (C) 2020.02.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <linux/miscdevice.h>
#include <linux/syscore_ops.h>
#include <linux/preempt.h>
#include <linux/debugfs.h>
#include "kvm/internal.h"
#include "kvm/async_pf.h"

static cpumask_var_t cpus_hardware_enabled_bs;
static bool largepages_enabled_bs = true;

struct kmem_cache *kvm_vcpu_cache_bs;
EXPORT_SYMBOL_GPL(kvm_vcpu_cache_bs);

struct dentry *kvm_debugfs_dir_bs;
EXPORT_SYMBOL_GPL(kvm_debugfs_dir_bs);

static int kvm_debugfs_num_entries_bs;

static __read_mostly struct preempt_ops kvm_preempt_ops_bs;

static long kvm_no_compat_ioctl_bs(struct file *file, unsigned int ioctl,
		unsigned long arg)
{
	return -EINVAL;
}

#define KVM_COMPAT_BS(c)	.compat_ioctl = kvm_no_compat_ioctl_bs

static int kvm_starting_cpu_bs(unsigned int cpu)
{
	BS_DUP();
	return 0;
}

static int kvm_dying_cpu_bs(unsigned int cpu)
{
	BS_DUP();
	return 0;
}

static int kvm_reboot_bs(struct notifier_block *notifier, unsigned long val,
				void *v)
{
	BS_DUP();
	return 0;
}

static long kvm_dev_ioctl_bs(struct file *filp,
			unsigned int iotcl, unsigned long arg)
{
	BS_DUP();
	return 0;
}

static int kvm_vm_release_bs(struct inode *inode, struct file *filp)
{
	BS_DUP();
	return 0;
}

static long kvm_vm_ioctl_bs(struct file *filp,
				unsigned int ioctl, unsigned long arg)
{
	BS_DUP();
	return 0;
}

static int kvm_vcpu_release_bs(struct inode *inode, struct file *filp)
{
	BS_DUP();
	return 0;
}

static long kvm_vcpu_ioctl_bs(struct file *filp,
			unsigned int ioctl, unsigned long arg)
{
	BS_DUP();
	return 0;
}

static int kvm_vcpu_mmap_bs(struct file *filp, struct vm_area_struct *vma)
{
	BS_DUP();
	return 0;
}

static int kvm_suspend_bs(void)
{
	BS_DUP();
	return 0;
}

static void kvm_resume_bs(void)
{
	BS_DUP();
}

static void kvm_sched_in_bs(struct preempt_notifier *pn, int cpu)
{
	BS_DUP();
}

static void kvm_sched_out_bs(struct preempt_notifier *pn,
				struct task_struct *next)
{
	BS_DUP();
}

static void kvm_init_debug_bs(void)
{
	struct kvm_stats_debugfs_item *p;

	kvm_debugfs_dir_bs = debugfs_create_dir("kvm_bs", NULL);

	kvm_debugfs_num_entries_bs = 0;
	for (p = debugfs_entries_bs; p->name; 
					++p, kvm_debugfs_num_entries_bs++) {
		debugfs_create_file(p->name, 0644, kvm_debugfs_dir_bs,
					(void *)(long)p->offset,
					stat_fops_bs[p->kind]);
	}

}

static int vcpu_stat_get_bs(void *_offset, u64 *val)
{
	BS_DUP();
	return 0;
}

static int vcpu_stat_clear_bs(void *_offset, u64 val)
{
	BS_DUP();
	return 0;
}

static int vm_stat_get_bs(void *_offset, u64 *val)
{
	BS_DUP();
	return 0;
}

static int vm_stat_clear_bs(void *_offset, u64 val)
{
	BS_DUP();
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(vcpu_stat_fops_bs, vcpu_stat_get_bs, 
				vcpu_stat_clear_bs, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(vm_stat_fops_bs, vm_stat_get_bs,
				vm_stat_clear_bs, "%llu\n");

static struct notifier_block kvm_reboot_notifier_bs = {
	.notifier_call = kvm_reboot_bs,
	.priority = 0,
};

static struct file_operations kvm_chardev_ops_bs = {
	.unlocked_ioctl = kvm_dev_ioctl_bs,
	.llseek 	= noop_llseek,
	KVM_COMPAT_BS(kvm_dev_ioctl_bs),
};

static struct file_operations kvm_vm_fops_bs = {
	.release	= kvm_vm_release_bs,
	.unlocked_ioctl	= kvm_vm_ioctl_bs,
	.llseek		= noop_llseek,
	KVM_COMPAT_BS(kvm_vm_compat_ioctl_bs),
};

static struct file_operations kvm_vcpu_fops_bs = {
	.release	= kvm_vcpu_release_bs,
	.unlocked_ioctl	= kvm_vcpu_ioctl_bs,
	.mmap		= kvm_vcpu_mmap_bs,
	.llseek		= noop_llseek,
	KVM_COMPAT_BS(kvm_vcpu_compat_ioctl),
};

static struct miscdevice kvm_dev_bs = {
	KVM_MINOR,
	"kvm_bs",
	&kvm_chardev_ops_bs,
};

static struct syscore_ops kvm_syscore_ops_bs = {
	.suspend = kvm_suspend_bs,
	.resume  = kvm_resume_bs,
};

static const struct file_operations *stat_fops_bs[] = {
	[KVM_STAT_VCPU]	= &vcpu_stat_fops_bs,
	[KVM_STAT_VM]	= &vm_stat_fops_bs,
};

int kvm_init_bs(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
				struct module *module)
{
	int cpu;
	int r;

	r = kvm_arch_init_bs(opaque);
	if (r)
		goto out_fail;

	/*
	 * kvm_arch_init makes sure there's at most one caller
	 * for architectures that support multiple implementations,
	 * like intel and amd on x86.
	 * kvm_arch_init must be called before kvm_irqfd_init to avoid
	 * creating conflicts on case kvm is already setup for another
	 * implementation.
	 */
	r = kvm_irqfd_init_bs();
	if (r)
		goto out_irqfd;

	if (!zalloc_cpumask_var(&cpus_hardware_enabled_bs, GFP_KERNEL)) {
		r = -ENOMEM;
		goto out_free_0;
	}

	r = kvm_arch_hardware_setup_bs();
	if (r < 0)
		goto out_free_0a;

	for_each_online_cpu(cpu) {
		smp_call_function_single(cpu,
			kvm_arch_check_processor_compat_bs,
			&r, 1);
		if (r < 0)
			goto out_free_1;
	}

	r = cpuhp_setup_state_nocalls(CPUHP_AP_KVM_STARTING, 
			"kvm_bs/cpu:starting", kvm_starting_cpu_bs,
						kvm_dying_cpu_bs);
	if (r)
		goto out_free_2;
	register_reboot_notifier(&kvm_reboot_notifier_bs);

	/* A kmem cache lets us meet the alignment requirements of fx_save */
	if (!vcpu_align)
		vcpu_align = __alignof__(struct kvm_vcpu);
	kvm_vcpu_cache_bs =
		kmem_cache_create_usercopy("kvm_vcpu_bs",
					vcpu_size,
					vcpu_align,
					SLAB_ACCOUNT,
					offsetof(struct kvm_vcpu, arch),
					sizeof_field(struct kvm_vcpu, arch),
					NULL);
	if (!kvm_vcpu_cache_bs) {
		r = -ENOMEM;
		goto out_free_3;
	}

	r = kvm_async_pf_init_bs();
	if (r)
		goto out_free;

	kvm_chardev_ops_bs.owner = module;
	kvm_vm_fops_bs.owner = module;
	kvm_vcpu_fops_bs.owner = module;

	r = misc_register(&kvm_dev_bs);
	if (r) {
		pr_err("kvm: misc device register failed\n");
		goto out_unreg;
	}

	register_syscore_ops(&kvm_syscore_ops_bs);

	kvm_preempt_ops_bs.sched_in = kvm_sched_in_bs;
	kvm_preempt_ops_bs.sched_out = kvm_sched_out_bs;

	kvm_init_debug_bs();

	r = kvm_vfio_ops_init_bs();
	WARN_ON(r);

	return 0;
out_unreg:
	BS_DUP();
out_free:
	BS_DUP();
out_free_3:
	BS_DUP();
out_free_2:
	BS_DUP();
out_free_1:
	BS_DUP();
out_free_0a:
	BS_DUP();
out_free_0:
	BS_DUP();
out_irqfd:
	BS_DUP();
out_fail:
	return r;
}
EXPORT_SYMBOL_GPL(kvm_init_bs);

void kvm_disable_largepages_bs(void)
{
	largepages_enabled_bs = false;
}
EXPORT_SYMBOL_GPL(kvm_disable_largepages_bs);

static struct kvm_device_ops *kvm_device_ops_table_bs[KVM_DEV_TYPE_MAX] = { };

int kvm_register_device_ops_bs(struct kvm_device_ops *ops, u32 type)
{
	if (type >= ARRAY_SIZE(kvm_device_ops_table_bs))
		return -ENOSPC;

	if (kvm_device_ops_table_bs[type] != NULL)
		return -EEXIST;

	kvm_device_ops_table_bs[type] = ops;
	return 0;
}
