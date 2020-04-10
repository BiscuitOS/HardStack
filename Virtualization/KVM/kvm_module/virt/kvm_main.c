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
#include <linux/sched/mm.h>
#include <linux/anon_inodes.h>
#include "kvm/internal.h"
#include "kvm/async_pf.h"

/* Wrost case buffer size needed for holding an integer. */
#define ITOA_MAX_LEN	12

MODULE_AUTHOR("Qumranet");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BiscuitOS KVM Project");

#define KVM_EVENT_CREATE_VM	0
#define KVM_EVENT_DESTROY_VM	1
static unsigned long long kvm_createvm_count_bs;
static unsigned long long kvm_active_vms_bs;

static cpumask_var_t cpus_hardware_enabled_bs;
static bool largepages_enabled_bs = true;

struct kmem_cache *kvm_vcpu_cache_bs;
EXPORT_SYMBOL_GPL(kvm_vcpu_cache_bs);

struct dentry *kvm_debugfs_dir_bs;
EXPORT_SYMBOL_GPL(kvm_debugfs_dir_bs);

static int kvm_debugfs_num_entries_bs;

/*
 * Ordering of locks:
 *
 *      kvm->lock --> kvm->slots_lock --> kvm->irq_lock
 */

DEFINE_SPINLOCK(kvm_lock_bs);
static DEFINE_RAW_SPINLOCK(kvm_count_lock_bs);
LIST_HEAD(vm_list_bs);

static cpumask_var_t cpus_hardware_enabled_bs;
static int kvm_usage_count_bs;
static atomic_t hardware_enable_failed_bs;

static int kvm_usage_count_bs;

static __read_mostly struct preempt_ops kvm_preempt_ops_bs;
static struct file_operations kvm_vm_fops_bs;
static struct miscdevice kvm_dev_bs;

static void hardware_enable_nolock_bs(void *junk)
{
	int cpu = raw_smp_processor_id();
	int r;

	if (cpumask_test_cpu(cpu, cpus_hardware_enabled_bs))
		return;

	cpumask_set_cpu(cpu, cpus_hardware_enabled_bs);

	r = kvm_arch_hardware_enable_bs();

	if (r)
		BS_DUP();
}

static int hardware_enable_all_bs(void)
{
	int r = 0;

	raw_spin_lock(&kvm_count_lock_bs);

	kvm_usage_count_bs++;
	if (kvm_usage_count_bs == 1) {
		atomic_set(&hardware_enable_failed_bs, 0);
		on_each_cpu(hardware_enable_nolock_bs, NULL, 1);

		if (atomic_read(&hardware_enable_failed_bs)) {
			BS_DUP();
			r = -EBUSY;
		}
	}

	raw_spin_unlock(&kvm_count_lock_bs);

	return r;
}

static struct kvm_memslots *kvm_alloc_memslots_bs(void)
{
	int i;
	struct kvm_memslots *slots;

	slots = kvzalloc(sizeof(struct kvm_memslots), GFP_KERNEL);
	if (!slots)
		return NULL;

	for (i = 0; i < KVM_MEM_SLOTS_NUM; i++)
		slots->id_to_index[i] = slots->memslots[i].id = i;

	return slots;
}

static int
kvm_mmu_notifier_invalidate_range_start_bs(struct mmu_notifier *mn,
				const struct mmu_notifier_range *range)
{
	BS_DUP();
	return 0;
}

static void
kvm_mmu_notifier_invalidate_range_end_bs(struct mmu_notifier *mn,
				const struct mmu_notifier_range *range)
{
	BS_DUP();
}

static int kvm_mmu_notifier_clear_flush_young_bs(struct mmu_notifier *mn,
		struct mm_struct *mm, unsigned long start, unsigned long end)
{
	BS_DUP();
	return 0;
}

static int kvm_mmu_notifier_clear_young_bs(struct mmu_notifier *mn,
		struct mm_struct *mm, unsigned long start, unsigned long end)
{
	BS_DUP();
	return 0;
}

static int kvm_mmu_notifier_test_young_bs(struct mmu_notifier *mn,
				struct mm_struct *mm, unsigned long address)
{
	BS_DUP();
	return 0;
}

static void kvm_mmu_notifier_release_bs(struct mmu_notifier *mn,
						struct mm_struct *mm)
{
	BS_DUP();
}

static void kvm_mmu_notifier_change_pte_bs(struct mmu_notifier *mn,
		struct mm_struct *mm, unsigned long address, pte_t pte)
{
	BS_DUP();
}

static int vcpu_stat_get_per_vm_open_bs(struct inode *inode, struct file *file)
{
	BS_DUP();
	return 0;
}

static int kvm_debugfs_release_bs(struct inode *inode, struct file *file)
{
	BS_DUP();
	return 0;
}

static int vm_stat_get_per_vm_open_bs(struct inode *inode, struct file *file)
{
	BS_DUP();
	return 0;
}

static const struct mmu_notifier_ops kvm_mmu_notifier_ops_bs = {
	.invalidate_range_start	= kvm_mmu_notifier_invalidate_range_start_bs,
	.invalidate_range_end	= kvm_mmu_notifier_invalidate_range_end_bs,
	.clear_flush_young	= kvm_mmu_notifier_clear_flush_young_bs,
	.clear_young		= kvm_mmu_notifier_clear_young_bs,
	.test_young		= kvm_mmu_notifier_test_young_bs,
	.change_pte		= kvm_mmu_notifier_change_pte_bs,
	.release		= kvm_mmu_notifier_release_bs,
};

static const struct file_operations vcpu_stat_get_per_vm_fops_bs = {
	.owner			= THIS_MODULE,
	.open			= vcpu_stat_get_per_vm_open_bs,
	.release		= kvm_debugfs_release_bs,
	.read			= simple_attr_read,
	.write			= simple_attr_write,
	.llseek			= no_llseek,
};

static const struct file_operations vm_stat_get_per_vm_fops_bs = {
	.owner			= THIS_MODULE,
	.open			= vm_stat_get_per_vm_open_bs,
	.release		= kvm_debugfs_release_bs,
	.read			= simple_attr_read,
	.write			= simple_attr_write,
	.llseek			= no_llseek,
};

static const struct file_operations *stat_fops_per_vm_bs[] = {
	[KVM_STAT_VCPU]		= &vcpu_stat_get_per_vm_fops_bs,
	[KVM_STAT_VM]		= &vm_stat_get_per_vm_fops_bs,
};

static int kvm_init_mmu_notifier_bs(struct kvm *kvm)
{
	kvm->mmu_notifier.ops = &kvm_mmu_notifier_ops_bs;
	return mmu_notifier_register(&kvm->mmu_notifier, current->mm);
}

static struct kvm *kvm_create_vm_bs(unsigned long type)
{
	int r, i;
	struct kvm *kvm = kvm_arch_alloc_vm_bs();

	if (!kvm)
		return ERR_PTR(-ENOMEM);

	spin_lock_init(&kvm->mmu_lock);
	mmgrab(current->mm);
	kvm->mm = current->mm;
	kvm_eventfd_init_bs(kvm);
	mutex_init(&kvm->lock);
	mutex_init(&kvm->irq_lock);
	mutex_init(&kvm->slots_lock);
	refcount_set(&kvm->users_count, 1);
	INIT_LIST_HEAD(&kvm->devices);

	r = kvm_arch_init_vm_bs(kvm, type);
	if (r)
		goto out_err_no_disable;

	r = hardware_enable_all_bs();
	if (r)
		goto out_err_no_disable;

#ifdef CONFIG_HAVE_KVM_IRQFD
	INIT_HLIST_HEAD(&kvm->irq_ack_notifier_list);
#endif

	BUILD_BUG_ON(KVM_MEM_SLOTS_NUM > SHRT_MAX);

	r = -ENOMEM;
	for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++) {
		struct kvm_memslots *slots = kvm_alloc_memslots_bs();

		if (!slots)
			goto out_err_no_srcu;

		/*
		 * Generations must be different for each address space.
		 * Init kvm generation close to the maximum to easily test
		 * the code of handling generation number wrap-around.
		 */
		slots->generation = i * 2 - 150;
		rcu_assign_pointer(kvm->memslots[i], slots);
	}

	if (init_srcu_struct(&kvm->srcu))
		goto out_err_no_srcu;
	if (init_srcu_struct(&kvm->irq_srcu))
		goto out_err_no_irq_srcu;
	for (i = 0; i < KVM_NR_BUSES; i++) {
		rcu_assign_pointer(kvm->buses[i],
			kzalloc(sizeof(struct kvm_io_bus), GFP_KERNEL));
		if (!kvm->buses[i])
			goto out_err;
	}

	r = kvm_init_mmu_notifier_bs(kvm);
	if (r)
		goto out_err;

	spin_lock(&kvm_lock_bs);
	list_add(&kvm->vm_list, &vm_list_bs);
	spin_unlock(&kvm_lock_bs);

	preempt_notifier_inc();

	return kvm;

out_err:
	BS_DUP();
out_err_no_irq_srcu:
	BS_DUP();
out_err_no_srcu:
	BS_DUP();
out_err_no_disable:
	BS_DUP();
	return ERR_PTR(r);
}

static int kvm_create_vm_debugfs_bs(struct kvm *kvm, int fd)
{
	char dir_name[ITOA_MAX_LEN * 2];
	struct kvm_stat_data *stat_data;
	struct kvm_stats_debugfs_item *p;

	if (!debugfs_initialized())
		return 0;

	snprintf(dir_name, sizeof(dir_name), "%d-%d",
						task_pid_nr(current), fd);
	kvm->debugfs_dentry = debugfs_create_dir(dir_name, 
						kvm_debugfs_dir_bs);
	kvm->debugfs_stat_data = kcalloc(kvm_debugfs_num_entries_bs,
					sizeof(*kvm->debugfs_stat_data),
					GFP_KERNEL);
	if (!kvm->debugfs_stat_data)
		return -ENOMEM;

	for (p = debugfs_entries_bs; p->name; p++) {
		stat_data = kzalloc(sizeof(*stat_data),  GFP_KERNEL);
		if (!stat_data)
			return -ENOMEM;

		stat_data->kvm = kvm;
		stat_data->offset = p->offset;
		kvm->debugfs_stat_data[p - debugfs_entries_bs] = stat_data;
		debugfs_create_file(p->name, 0644, kvm->debugfs_dentry,
				   stat_data, stat_fops_per_vm_bs[p->kind]);
	}
	return 0;
}

static void kvm_uevent_notify_change_bs(unsigned int type, struct kvm *kvm)
{
	struct kobj_uevent_env *env;
	unsigned long long created, active;

	if (!kvm_dev_bs.this_device || !kvm)
		return;

	spin_lock(&kvm_lock_bs);
	if (type == KVM_EVENT_CREATE_VM) {
		kvm_createvm_count_bs++;
		kvm_active_vms_bs++;
	} else if (type == KVM_EVENT_DESTROY_VM) {
		kvm_active_vms_bs--;
	}
	created = kvm_createvm_count_bs;
	active = kvm_active_vms_bs;
	spin_unlock(&kvm_lock_bs);

	env = kzalloc(sizeof(*env), GFP_KERNEL);
	if (!env)
		return;

	add_uevent_var(env, "CREATED=%llu", created);
	add_uevent_var(env, "COUNT=%llu", active);

	if (type == KVM_EVENT_CREATE_VM) {
		add_uevent_var(env, "EVENT=create");
		kvm->userspace_pid = task_pid_nr(current);
	} else if (type == KVM_EVENT_DESTROY_VM) {
		add_uevent_var(env, "EVENT=destroy");
	}
	add_uevent_var(env, "PID=%d", kvm->userspace_pid);

	if (!IS_ERR_OR_NULL(kvm->debugfs_dentry)) {
		char *tmp, *p = kmalloc(PATH_MAX, GFP_KERNEL);

		if (p) {
			tmp = dentry_path_raw(kvm->debugfs_dentry, 
								p, PATH_MAX);
			if (!IS_ERR(tmp))
				add_uevent_var(env, "STATS_PATH=%s", tmp);
			kfree(p);
		}
	}
	/* no need for checks, since we are adding at most only 5 keys */
	env->envp[env->envp_idx++] = NULL;
	kobject_uevent_env(&kvm_dev_bs.this_device->kobj, 
						KOBJ_CHANGE, env->envp);
	kfree(env);
}

static int kvm_dev_ioctl_create_vm_bs(unsigned long type)
{
	int r;
	struct kvm *kvm;
	struct file *file;

	kvm = kvm_create_vm_bs(type);
	if (IS_ERR(kvm))
		return PTR_ERR(kvm);

#ifdef CONFIG_KVM_MMIO
	r = kvm_coalesced_mmio_init_bs(kvm);
	if (r < 0)
		goto put_kvm;
#endif
	r = get_unused_fd_flags(O_CLOEXEC);
	if (r < 0)
		goto put_kvm;

	file = anon_inode_getfile("kvm-vm-bs", &kvm_vm_fops_bs, kvm, O_RDWR);
	if (IS_ERR(file)) {
		BS_DUP();
		goto put_kvm;
	}

	/*
	 * Don't call kvm_put_kvm anymore at this point; file->f_op is
	 * already set, with ->release() being kvm_vm_release(). In error
	 * cases it will be called by the final fput(file) and will take
	 * care of doing kvm_put_kvm(kvm).
	 */
	if (kvm_create_vm_debugfs_bs(kvm, r) < 0) {
		BS_DUP();
		return -ENOMEM;
	}
	kvm_uevent_notify_change_bs(KVM_EVENT_CREATE_VM, kvm);

	fd_install(r, file);

	return r;

put_kvm:
	BS_DUP();
	return r;
}

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
			unsigned int ioctl, unsigned long arg)
{
	long r = -EINVAL;

	switch (ioctl) {
	case KVM_GET_API_VERSION:
		BS_DUP();
		break;
	case KVM_CREATE_VM:
		BS_DONE();
		r = kvm_dev_ioctl_create_vm_bs(arg);
		break;
	case KVM_CHECK_EXTENSION:
		BS_DUP();
		break;
	case KVM_GET_VCPU_MMAP_SIZE:
		BS_DUP();
		break;
	case KVM_TRACE_ENABLE:
	case KVM_TRACE_PAUSE:
	case KVM_TRACE_DISABLE:
		BS_DUP();
		break;
	default:
		BS_DUP();
		break;
	}
out:
	return r;
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
