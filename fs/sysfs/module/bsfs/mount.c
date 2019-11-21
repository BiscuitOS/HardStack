/*
 * mount
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/exportfs.h>

#include "linux/bsfs.h"

struct kmem_cache *bsfs_node_cache;

void bsfs_init(void)
{
	bsfs_node_cache = kmem_cache_create("bsfs_node_cache",
				sizeof(struct bsfs_node),
				0,
				SLAB_PANIC | SLAB_TYPESAFE_BY_RCU,
				NULL);
}

static int bsfs_test_super(struct super_block *sb, void *data)
{
	struct bsfs_super_info *sb_info = bsfs_info(sb);
	struct bsfs_super_info *info = data;

	return sb_info->root == info->root && sb_info->ns == info->ns;
}

static int bsfs_set_super(struct super_block *sb, void *data)
{
	int error;
	error = set_anon_super(sb, data);
	if (!error)
		sb->s_fs_info = data;
	return error;
}

static int bsfs_sop_remount_fs(struct super_block *sb, int *flags, char *data)
{
	struct bsfs_root *root = bsfs_info(sb)->root;
	struct bsfs_syscall_ops *scops = root->syscall_ops;

	if (scops && scops->remount_fs)
		return scops->remount_fs(root, flags, data);
	return 0;
}

static int bsfs_sop_show_options(struct seq_file *sf, struct dentry *dentry)
{
	struct bsfs_root *root = bsfs_root(bsfs_dentry_node(dentry));
	struct bsfs_syscall_ops *scops = root->syscall_ops;

	if (scops && scops->show_options)
		return scops->show_options(sf, root);
	return 0;
}

static int bsfs_sop_show_path(struct seq_file *sf, struct dentry *dentry)
{
	struct bsfs_node *node = bsfs_dentry_node(dentry);
	struct bsfs_root *root = bsfs_root(node);
	struct bsfs_syscall_ops *scops = root->syscall_ops;

	if (scops && scops->show_path)
		return scops->show_path(sf, node, root);

	seq_dentry(sf, dentry, " \t\n\\");
	return 0;
}

static struct inode *bsfs_fh_get_inode(struct super_block *sb,
		u64 ino, u32 generation)
{
	struct bsfs_super_info *info = bsfs_info(sb);
	struct inode *inode;
	struct bsfs_node *kn;

	if (ino == 0)
		return ERR_PTR(-ESTALE);

	kn = bsfs_find_and_get_node_by_ino(info->root, ino);
	if (!kn)
		return ERR_PTR(-ESTALE);
	inode = bsfs_get_inode(sb, kn);
	bsfs_put(kn);
	if (!inode)
		return ERR_PTR(-ESTALE);

	if (generation && inode->i_generation != generation) {
		iput(inode);
		return ERR_PTR(-ESTALE);
	}
	return inode;
}

static struct dentry *bsfs_fh_to_parent(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
					bsfs_fh_get_inode);
}

static struct dentry *bsfs_get_parent_dentry(struct dentry *child)
{
	struct bsfs_node *kn = bsfs_dentry_node(child);

	return d_obtain_alias(bsfs_get_inode(child->d_sb, kn->parent));
}

static struct dentry *bsfs_fh_to_dentry(struct super_block *sb, 
			struct fid *fid, int fh_len, int fh_type)
{
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
					bsfs_fh_get_inode);
}

const struct super_operations bsfs_sops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.evict_inode	= bsfs_evict_inode,

	.remount_fs	= bsfs_sop_remount_fs,
	.show_options	= bsfs_sop_show_options,
	.show_path	= bsfs_sop_show_path,
};

static const struct export_operations bsfs_export_ops = {
	.fh_to_dentry	= bsfs_fh_to_dentry,
	.fh_to_parent	= bsfs_fh_to_parent,
	.get_parent	= bsfs_get_parent_dentry,
};

static int bsfs_fill_super(struct super_block *sb, unsigned long magic)
{
	struct bsfs_super_info *info = bsfs_info(sb);
	struct inode *inode;
	struct dentry *root;

	info->sb = sb;
	sb->s_iflags |= SB_I_NOEXEC | SB_I_NODEV;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = magic;
	sb->s_op = &bsfs_sops;
	sb->s_xattr = bsfs_xattr_handlers;
	if (info->root->flags & BSFS_ROOT_SUPPORT_EXPORTOP)
		sb->s_export_op = &bsfs_export_ops;
	sb->s_time_gran = 1;

	sb->s_shrink.seeks = 0;

	mutex_lock(&bsfs_mutex);
	inode = bsfs_get_inode(sb, info->root->kn);
	mutex_unlock(&bsfs_mutex);
	if (!inode) {
		pr_debug("bsfs: could not get root inode\n");
		return -ENOMEM;
	}

	root = d_make_root(inode);
	if (!root) {
		pr_debug("%s: could not get root dentry!\n", __func__);
	}
	sb->s_root = root;
	sb->s_d_op = &bsfs_dops;
	return 0;
}

struct dentry *bsfs_mount_ns(struct file_system_type *fs_type, int flags,
			struct bsfs_root *root, unsigned long magic,
			bool *new_sb_created, const void *ns)
{
	struct super_block *sb;
	struct bsfs_super_info *info;
	int error;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return ERR_PTR(-ENOMEM);

	info->root = root;
	info->ns = ns;
	INIT_LIST_HEAD(&info->node);

	sb = sget_userns(fs_type, bsfs_test_super, bsfs_set_super, flags,
			&init_user_ns, info);
	if (IS_ERR(sb) || sb->s_fs_info != info)
		kfree(info);
	if (IS_ERR(sb))
		return ERR_CAST(sb);

	if (new_sb_created)
		*new_sb_created = !sb->s_root;

	if (!sb->s_root) {
		struct bsfs_super_info *info = bsfs_info(sb);

		error = bsfs_fill_super(sb, magic);
		if (error) {
			deactivate_locked_super(sb);
			return ERR_PTR(error);
		}
		sb->s_flags |= SB_ACTIVE;

		mutex_lock(&bsfs_mutex);
		list_add(&info->node, &root->supers);
		mutex_unlock(&bsfs_mutex);
	}
	return dget(sb->s_root);
}

void bsfs_kill_sb(struct super_block *sb)
{
	struct bsfs_super_info *info = bsfs_info(sb);

	mutex_lock(&bsfs_mutex);
	list_del(&info->node);
	mutex_unlock(&bsfs_mutex);

	kill_anon_super(sb);
	kfree(info);
}

const void *bsfs_super_ns(struct super_block *sb)
{
	struct bsfs_super_info *info = bsfs_info(sb);

	return info->ns;
}
