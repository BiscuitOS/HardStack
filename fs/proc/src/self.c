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
#include <linux/pid_namespace.h>

#include "internal.h"

static unsigned self_inum_bs __ro_after_init;

void __init proc_self_init_bs(void)
{
	proc_alloc_inum_bs(&self_inum_bs);
}

static const char *proc_self_get_link_bs(struct dentry *dentry,
			struct inode *inode, struct delayed_call *done)
{
	BS_DUP();
	return NULL;
}

static const struct inode_operations proc_self_inode_operations_bs = {
	.get_link	= proc_self_get_link_bs,
};

int proc_setup_self_bs(struct super_block *s)
{
	struct inode *root_inode = d_inode(s->s_root);
	struct pid_namespace *ns = proc_pid_ns_bs(root_inode);
	struct dentry *self;

	inode_lock(root_inode);
	self = d_alloc_name(s->s_root, "self");
	if (self) {
		struct inode *inode = new_inode_pseudo(s);

		if (inode) {
			inode->i_ino = self_inum_bs;
			inode->i_mtime = inode->i_atime 
					= inode->i_ctime = current_time(inode);
			inode->i_mode = S_IFLNK | S_IRWXUGO;
			inode->i_uid = GLOBAL_ROOT_UID;
			inode->i_gid = GLOBAL_ROOT_GID;
			inode->i_op = &proc_self_inode_operations_bs;
			d_add(self, inode);
		} else {
			dput(self);
			self = ERR_PTR(-ENOMEM);
		}
	} else {
		self = ERR_PTR(-ENOMEM);
	}
	inode_unlock(root_inode);
	if (IS_ERR(self)) {
		pr_err("proc_fill_super: can't allocate /proc/self\n");
		return PTR_ERR(self);
	}
	ns->proc_self = self;
	return 0;
}
