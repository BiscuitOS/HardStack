/*
 * Proc filesytem
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_ns.h>
#include <linux/pid_namespace.h>
#include <linux/parser.h>
#include <linux/sched/stat.h>

#include "internal.h"

static int proc_root_getattr_bs(const struct path *path, struct kstat *stat,
		u32 request_mask, unsigned int query_flags)
{
	generic_fillattr(d_inode(path->dentry), stat);
	stat->nlink = proc_root_bs.nlink + nr_processes();
	return 0;
}

static struct dentry *proc_root_lookup_bs(struct inode *dir,
			struct dentry *dentry, unsigned int flags)
{
	if (!proc_pid_lookup_bs(dir, dentry, flags))
		return 0;
	return proc_lookup_bs(dir, dentry, flags);
}

static int proc_root_readdir_bs(struct file *file, struct dir_context *ctx)
{
	if (ctx->pos < FIRST_PROCESS_ENTRY) {
		int error = proc_readdir_bs(file, ctx);

		if (unlikely(error <= 0))
			return error;

		ctx->pos = FIRST_PROCESS_ENTRY;
	}
	return proc_pid_readdir_bs(file, ctx);
}

static void proc_kill_sb_bs(struct super_block *sb)
{
	BS_DUP();
}

static struct dentry *proc_mount_bs(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	struct pid_namespace *ns;

	if (flags & SB_KERNMOUNT) {
		ns = data;
		data = NULL;
	} else {
		ns = task_active_pid_ns(current);
	}

	return mount_ns(fs_type, flags, data, ns, ns->user_ns, 
						proc_fill_super_bs);
}

int proc_parse_options_bs(char *options, struct pid_namespace *pid)
{
	if (!options)
		return 1;

	BS_DUP();
	return 1;
}

int proc_remount_bs(struct super_block *sb, int *flags, char *data)
{
	BS_DUP();
	return 0;
}

/*
 * proc root can do almost nothing..
 */
static const struct inode_operations proc_root_inode_operations_bs = {
	.lookup		= proc_root_lookup_bs,
	.getattr	= proc_root_getattr_bs,
};

/*
 * The root /proc directory is special, as it has the
 * <pid> directoryies. Thus we don't use the generic
 * directory handling functions for that..
 */
static const struct file_operations proc_root_operations_bs = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_root_readdir_bs,
	.llseek		= generic_file_llseek,
};

/*
 * This is the root "inode" in the /proc tree..
 */
struct proc_dir_entry proc_root_bs = {
	.low_ino	= PROC_ROOT_INO,
	.namelen	= 8,
	.mode		= S_IFDIR | S_IRUGO | S_IXUGO,
	.nlink		= 2,
	.refcnt		= REFCOUNT_INIT(1),
	.proc_iops	= &proc_root_inode_operations_bs,
	.proc_fops	= &proc_root_operations_bs,
	.parent		= &proc_root_bs,
	.subdir		= RB_ROOT,
	.name		= "/proc_bs",
};

static struct file_system_type proc_fs_type_bs = {
	.name		= "proc_bs",
	.mount		= proc_mount_bs,
	.kill_sb	= proc_kill_sb_bs,
	.fs_flags	= FS_USERNS_MOUNT,
};

static int __init proc_root_init_bs(void)
{
	printk("Hello Proc.....\n\n\n\n");
	proc_init_kmemcache_bs();
	set_proc_pid_nlink_bs();
	proc_self_init_bs();
	proc_thread_self_init_bs();
	proc_symlink_bs("mounts_bs", NULL, "self/mounts_bs");

	proc_mkdir_bs("fs", NULL);
	proc_mkdir_bs("driver", NULL);
	proc_create_mount_point_bs("fs/nfsd_bs");
	proc_mkdir_bs("bus", NULL);
	proc_sys_init_bs();

	register_filesystem(&proc_fs_type_bs);
	printk("END...\n");
	return 0;
}
fs_initcall(proc_root_init_bs);
