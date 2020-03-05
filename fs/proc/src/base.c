/*
 * Proc filesytem -- proc base directory handling function
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/pid_namespace.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/ptrace.h>
#include <linux/wait.h>
#include <linux/security.h>
#include <linux/namei.h>

#include "internal.h"

static u8 nlink_tid_bs __ro_after_init;
static u8 nlink_tgid_bs __ro_after_init;

struct pid_entry {
	const char *name;
	unsigned int len;
	umode_t mode;
	const struct inode_operations *iop;
	const struct file_operations *fop;
	union proc_op op;
};

/*
 * Find the first task with tgid >= tgid
 */
struct tgid_iter {
	unsigned int tgid;
	struct task_struct *task;
};

#define TGID_OFFSET	(FIRST_PROCESS_ENTRY + 2)

#define NOD(NAME, MODE, IOP, FOP, OP) {				\
	.name	= (NAME),					\
	.len	= sizeof(NAME) - 1,				\
	.mode	= MODE,						\
	.iop	= IOP,						\
	.fop	= FOP,						\
	.op	= OP,						\
}

#define DIR(NAME, MODE, iops, fops)				\
	NOD(NAME, (S_IFDIR|(MODE)), &iops, &fops, {})

void pid_update_inode_bs(struct task_struct *task, struct inode *inode);
void task_dump_owner_bs(struct task_struct *task, umode_t mode,
						kuid_t *ruid, kgid_t *rgid);

int proc_setattr_bs(struct dentry *dentry, struct iattr *attr)
{
	BS_DUP();
	return 0;
}

/*
 * Count the number of hardlinks for the pid_entry table, excluding the .
 * and .. links.
 */
static unsigned int __init pid_entry_nlink_bs(const struct pid_entry *entries,
					unsigned int n)
{
	unsigned int i;
	unsigned int count;

	count = 2;
	for (i = 0; i < n; ++i) {
		if (S_ISDIR(entries[i].mode))
			++count;
	}
	return count;
}

/*
 * Tasks
 */
static const struct pid_entry tid_base_stuff_bs[] = {
	DIR(	"fd",
		S_IRUSR|S_IXUSR, 
		proc_fd_inode_operations_bs, 
		proc_fd_operations_bs
	),
};

/*
 * Thread groups
 */
static const struct pid_entry tgid_base_stuff_bs[] = {
	DIR(	"fd",
		S_IRUSR|S_IXUSR,
		proc_fd_inode_operations_bs,
		proc_fd_operations_bs
	),
};

void __init set_proc_pid_nlink_bs(void)
{
	nlink_tid_bs = pid_entry_nlink_bs(tid_base_stuff_bs,
					ARRAY_SIZE(tid_base_stuff_bs));
	nlink_tgid_bs = pid_entry_nlink_bs(tgid_base_stuff_bs,
					ARRAY_SIZE(tgid_base_stuff_bs));
}

static struct tgid_iter next_tgid_bs(struct pid_namespace *ns,
						struct tgid_iter iter)
{
	struct pid *pid;

	if (iter.task)
		put_task_struct(iter.task);
	rcu_read_lock();
retry:
	iter.task = NULL;
	pid = find_ge_pid(iter.tgid, ns);
	if (pid) {
		iter.tgid = pid_nr_ns(pid, ns);
		iter.task = pid_task(pid, PIDTYPE_PID);
		/* What we to know is if the pid we have find is the 
		 * pid of a thread_group_loader. Testing for task
		 * being a thread_group_loader is the obvious thing
		 * todo but there is a window when it fails, due to
		 * the pid transfer logic in de_thread.
		 *
		 * So we perform the straight forward test of seeing
		 * if the pid we have found is the pid of a thread
		 * group leader, and don't worry if the task we have
		 * found doesn't happen to be a thread group leader.
		 * As we don't care in the case of readdir.
		 */
		if (!iter.task || !has_group_leader_pid(iter.task)) {
			iter.tgid += 1;
			goto retry;
		}
		get_task_struct(iter.task);
	}
	rcu_read_unlock();
	return iter;
}

/*
 * May current process learn task's sched/cmdline info (for hide_pid_min=1)
 * or euid/egid (for hide_pid_min=2)?
 */
static bool has_pid_permissions_bs(struct pid_namespace *pid,
			struct task_struct *task, int hide_pid_min)
{
	if (pid->hide_pid < hide_pid_min)
		return true;
	if (in_group_p(pid->pid_gid))
		return true;
	return ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS);
}

static struct dentry *proc_tgid_base_lookup_bs(struct inode *dir,
				struct dentry *dentry, unsigned int flags)
{
	BS_DUP();
	return NULL;
}

int pid_getattr_bs(const struct path *path, struct kstat *stat,
				u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct pid_namespace *pid = proc_pid_ns_bs(inode);
	struct task_struct *task;

	generic_fillattr(inode, stat);

	stat->uid = GLOBAL_ROOT_UID;
	stat->gid = GLOBAL_ROOT_GID;
	rcu_read_lock();
	task = pid_task(proc_pid_bs(inode), PIDTYPE_PID);
	if (task) {
		if (!has_pid_permissions_bs(pid, task, HIDEPID_INVISIBLE)) {
			rcu_read_unlock();
			return -ENOENT;
		}
		task_dump_owner_bs(task, inode->i_mode, 
						&stat->uid, &stat->gid);
	}
	rcu_read_unlock();
	return 0;
}

static int proc_pid_permission_bs(struct inode *inode, int mask)
{
	struct pid_namespace *pid = proc_pid_ns_bs(inode);
	struct task_struct *task;
	bool has_perms;

	task = get_proc_task_bs(inode);
	if (!task)
		return -ESRCH;
	has_perms = has_pid_permissions_bs(pid, task, HIDEPID_NO_ACCESS);
	put_task_struct(task);

	if (!has_perms) {
		if (pid->hide_pid == HIDEPID_INVISIBLE) {
			return -ENOENT;
		}

		return -EPERM;
	}
	return generic_permission(inode, mask);
}

static int proc_tgid_base_readdir_bs(struct file *file,
						struct dir_context *ctx)
{
	BS_DUP();
	return 0;
}

static int pid_revalidate_bs(struct dentry *dentry, unsigned int flags)
{
	struct inode *inode;
	struct task_struct *task;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	inode = d_inode(dentry);
	task = get_proc_task_bs(inode);

	if (task) {
		pid_update_inode_bs(task, inode);
		put_task_struct(task);
		return 1;
	}

	return 0;
}

static inline bool proc_inode_is_dead_bs(struct inode *inode)
{
	return !proc_pid_bs(inode)->tasks[PIDTYPE_PID].first;
}

int pid_delete_dentry_bs(const struct dentry *dentry)
{
	/* IS the task we represent dead?
	 * If so, then don't put the dentry on the lru list,
	 * kill it immediately.
	 */
	return proc_inode_is_dead_bs(d_inode(dentry));
}

static const struct inode_operations proc_tgid_base_inode_operations_bs = {
	.lookup		= proc_tgid_base_lookup_bs,
	.getattr	= pid_getattr_bs,
	.setattr	= proc_setattr_bs,
	.permission	= proc_pid_permission_bs,
};

static const struct file_operations proc_tgid_base_operations_bs = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_tgid_base_readdir_bs,
	.llseek		= generic_file_llseek,
};

static struct dentry_operations pid_dentry_operations_bs = {
	.d_revalidate	= pid_revalidate_bs,
	.d_delete	= pid_delete_dentry_bs,
};

static const struct inode_operations proc_def_inode_operations_bs = {
	.setattr	= proc_setattr_bs,
};

/* Lookups */

/*
 * Fill a directory entry.
 *
 * If possible create the dcache entry and derive our inode number and
 * file type from dcache entry.
 *
 * Since all of the proc_bs inode numbers are dynamically generated, the inode
 * number do not exist unitl the inode is cache. This means creating the
 * the dcache entry in readdir is necessary to keep the inode numbers
 * reported by readdir in sync with the inode numbers reported by stat.
 */
bool proc_fill_cache_bs(struct file *file, struct dir_context *ctx,
	const char *name, unsigned int len, instantiate_t instantiate,
	struct task_struct *task, const void *ptr)
{
	struct dentry *child, *dir = file->f_path.dentry;
	struct qstr qname = QSTR_INIT(name, len);
	struct inode *inode;
	unsigned type = DT_UNKNOWN;
	ino_t ino = 1;
	
	child = d_hash_and_lookup(dir, &qname);
	if (!child) {
		DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wq);

		child = d_alloc_parallel(dir, &qname, &wq);
		if (IS_ERR(child))
			goto end_instantiate;
		if (d_in_lookup(child)) {
			struct dentry *res;

			res = instantiate(child, task, ptr);
			d_lookup_done(child);
			if (unlikely(res)) {
				dput(child);
				child = res;
				if (IS_ERR(child))
					goto end_instantiate;
			}
		}
	}
	inode = d_inode(child);
	ino = inode->i_ino;
	type = inode->i_mode >> 12;
	dput(child);
end_instantiate:
	return dir_emit(ctx, name, len, ino, type);
}

void task_dump_owner_bs(struct task_struct *task, umode_t mode, 
						kuid_t *ruid, kgid_t *rgid)
{
	/* Depending on the state of dumpable compute who should own a
	 * proc file for a task.
	 */
	const struct cred *cred;
	kuid_t uid;
	kgid_t gid;

	if (unlikely(task->flags & PF_KTHREAD)) {
		*ruid = GLOBAL_ROOT_UID;
		*rgid = GLOBAL_ROOT_GID;
		return;
	}

	/* Default to the task effective ownership */
	rcu_read_lock();
	cred = __task_cred(task);
	uid = cred->euid;
	gid = cred->egid;
	rcu_read_unlock();

	if (mode != (S_IFDIR | S_IRUGO | S_IXUGO)) {
		struct mm_struct *mm;

		task_lock(task);
		mm = task->mm;
		/* Make non-dumpable tasks owned by some root */
		if (mm) {
			if (get_dumpable(mm) != SUID_DUMP_USER) {
				struct user_namespace *user_ns = mm->user_ns;

				uid = make_kuid(user_ns, 0);
				if (!uid_valid(uid))
					uid = GLOBAL_ROOT_UID;

				gid = make_kgid(user_ns, 0);
				if (!gid_valid(gid))
					gid = GLOBAL_ROOT_GID;
			}
		} else {
			uid = GLOBAL_ROOT_UID;
			gid = GLOBAL_ROOT_GID;
		}
		task_unlock(task);
	}
	*ruid = uid;
	*rgid = gid;
}

/*
 * Set <pid>/... inode ownership (can change due to setuid(), etc. )
 */
void pid_update_inode_bs(struct task_struct *task, struct inode *inode)
{
	task_dump_owner_bs(task, inode->i_mode, &inode->i_uid, &inode->i_gid);

	inode->i_mode &= ~(S_ISUID | S_ISGID);
	security_task_to_inode(task, inode);
}

struct inode *proc_pid_make_inode_bs(struct super_block *sb,
				struct task_struct *task, umode_t mode)
{
	struct inode *inode;
	struct proc_inode *ei;

	/* We need a new inode */

	inode = new_inode(sb);
	if (!inode)
		goto out;

	/* Common stuff */
	ei = PROC_I_BS(inode);
	inode->i_mode = mode;
	inode->i_ino = get_next_ino();
	inode->i_mtime = inode->i_atime = 
				inode->i_ctime = current_time(inode);
	inode->i_op = &proc_def_inode_operations_bs;

	/*
	 * grab the reference to task.
	 */
	ei->pid = get_task_pid(task, PIDTYPE_PID);
	if (!ei->pid)
		goto out_unlock;

	task_dump_owner_bs(task, 0, &inode->i_uid, &inode->i_gid);
	security_task_to_inode(task, inode);

out:
	return inode;

out_unlock:
	iput(inode);
	return NULL;
}

static struct dentry *proc_pid_instantiate_bs(struct dentry *dentry,
			struct task_struct *task, const void *ptr)
{
	struct inode *inode;

	inode = proc_pid_make_inode_bs(dentry->d_sb, task, S_IFDIR | 
					S_IRUGO | S_IXUGO);
	if (!inode)
		return ERR_PTR(-ENOENT);

	inode->i_op = &proc_tgid_base_inode_operations_bs;
	inode->i_fop = &proc_tgid_base_operations_bs;
	inode->i_flags |= S_IMMUTABLE;

	set_nlink(inode, nlink_tgid_bs);
	pid_update_inode_bs(task, inode);

	d_set_d_op(dentry, &pid_dentry_operations_bs);
	return d_splice_alias(inode, dentry);
}

struct dentry *proc_pid_lookup_bs(struct inode *dir, struct dentry *dentry,
						unsigned int flags)
{
	struct task_struct *task;
	unsigned tgid;
	struct pid_namespace *ns;
	struct dentry *result = ERR_PTR(-ENOENT);

	tgid = name_to_int_bs(&dentry->d_name);
	if (tgid == ~0U)
		goto out;

	ns = dentry->d_sb->s_fs_info;
	rcu_read_lock();
	task = find_task_by_pid_ns(tgid, ns);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		goto out;

	result = proc_pid_instantiate_bs(dentry, task, NULL);
	put_task_struct(task);

out:
	return result;
}

/* for the /proc_bs/ directory itself, after non-process stuff has been done */
int proc_pid_readdir_bs(struct file *file, struct dir_context *ctx)
{
	struct tgid_iter iter;
	struct pid_namespace *ns = proc_pid_ns(file_inode(file));
	loff_t pos = ctx->pos;

	if (pos >= PID_MAX_LIMIT + TGID_OFFSET)
		return 0;

	if (pos >= TGID_OFFSET - 2) {
		struct inode *inode = d_inode(ns->proc_self);

		if (!dir_emit(ctx, "self", 4, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	if (pos == TGID_OFFSET - 1) {
		struct inode *inode = d_inode(ns->proc_thread_self);

		if (!dir_emit(ctx, "thread-self", 11, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	iter.tgid = pos - TGID_OFFSET;
	iter.task = NULL;
	for (iter = next_tgid_bs(ns, iter);
		iter.task;
		iter.tgid += 1, iter = next_tgid_bs(ns, iter)) {
		char name[10 + 1];
		unsigned int len;

		cond_resched();
		if (!has_pid_permissions_bs(ns, iter.task, HIDEPID_INVISIBLE))
			continue;

		len = snprintf(name, sizeof(name), "%u", iter.tgid);
		ctx->pos = iter.tgid + TGID_OFFSET;
		if (!proc_fill_cache_bs(file, ctx, name, len,
				proc_pid_instantiate_bs, iter.task, NULL)) {

		}
	}
	return 0;
}
