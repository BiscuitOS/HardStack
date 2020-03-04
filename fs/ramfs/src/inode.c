/*
 * Ramfs filesytem
 *
 * (C) 2020.03.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/seq_file.h>

#include "internal.h"

struct ramfs_mount_opts {
	umode_t mode;
};

struct ramfs_fs_info {
	struct ramfs_mount_opts mount_opts;
};

#define RAMFS_DEFAULT_MODE	0755

struct inode *ramfs_get_inode_bs(struct super_block *sb,
			const struct inode *dir, umode_t mode, dev_t dev);

static int ramfs_parse_options_bs(char *data, struct ramfs_mount_opts *opts)
{
	char *p;

	while ((p = strsep(&data, ",")) != NULL) {
		BS_DUP();
	}
	return 0;
}

/*
 * File creation. Allocate an inode, and we're done..
 */
/* SMP-safe */
static int
ramfs_mknod_bs(struct inode *dir, struct dentry *dentry, umode_t mode,
						dev_t dev)
{
	struct inode *inode = ramfs_get_inode_bs(dir->i_sb, dir, mode, dev);
	int error = -ENOSPC;

	if (inode) {
		d_instantiate(dentry, inode);
		dget(dentry);	/* Extra count - pin the dentry in core */
		error = 0;
		dir->i_mtime = dir->i_ctime = current_time(dir);
	}
	return error;
}

static int ramfs_create_bs(struct inode *dir, struct dentry *dentry,
					umode_t mode, bool excl)
{
	return ramfs_mknod_bs(dir, dentry, mode | S_IFREG, 0);
}

static int ramfs_symlink_bs(struct inode *dir, struct dentry *dentry,
				const char *symname)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = ramfs_get_inode_bs(dir->i_sb, dir, S_IFLNK | S_IRWXUGO, 0);
	if (inode) {
		int l = strlen(symname) + 1;

		error = page_symlink(inode, symname, l);
		if (!error) {
			d_instantiate(dentry, inode);
			dget(dentry);
			dir->i_mtime = dir->i_ctime = current_time(dir);
		} else
			iput(inode);
	}
	return error;
}

static int ramfs_mkdir_bs(struct inode *dir, struct dentry *dentry,
					umode_t mode)
{
	int retval = ramfs_mknod_bs(dir, dentry, mode | S_IFDIR, 0);

	if (!retval)
		inc_nlink(dir);
	return retval;
}

/*
 * Display the mount options in /proc/mounts.
 */
static int ramfs_show_options_bs(struct seq_file *m, struct dentry *root)
{
	struct ramfs_fs_info  *fsi = root->d_sb->s_fs_info;

	if (fsi->mount_opts.mode != RAMFS_DEFAULT_MODE)
		seq_printf(m, ",mode=%o", fsi->mount_opts.mode);
	return 0;
}

static const struct super_operations ramfs_ops_bs = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= ramfs_show_options_bs,
};

static const struct address_space_operations ramfs_aops_bs = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
	.set_page_dirty	= __set_page_dirty_no_writeback,
};

static const struct inode_operations ramfs_dir_inode_operations_bs = {
	.create		= ramfs_create_bs,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
	.symlink	= ramfs_symlink_bs,
	.mkdir		= ramfs_mkdir_bs,
	.rmdir		= simple_rmdir,
	.mknod		= ramfs_mknod_bs,
	.rename		= simple_rename,
};

struct inode *ramfs_get_inode_bs(struct super_block *sb,
			const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode *inode = new_inode(sb);

	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_mapping->a_ops = &ramfs_aops_bs;
		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
		mapping_set_unevictable(inode->i_mapping);
		inode->i_atime = inode->i_mtime =
					inode->i_ctime = current_time(inode);
		switch (mode & S_IFMT) {
		default:
			BS_DUP();
			break;
		case S_IFREG:
			inode->i_op = &ramfs_file_inode_operations_bs;
			inode->i_fop = &ramfs_file_operations_bs;
			break;
		case S_IFDIR:
			inode->i_op = &ramfs_dir_inode_operations_bs;
			inode->i_fop = &simple_dir_operations;

			/*
			 * Directory inodes start off with i_nlink == 2 (
			 * for "." entry)
			 */
			inc_nlink(inode);
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			inode_nohighmem(inode);
			break;
		}
	}
	return inode;
}

int ramfs_fill_super_bs(struct super_block *sb, void *data, int silent)
{
	struct ramfs_fs_info *fsi;
	struct inode *inode;
	int err;

	fsi = kzalloc(sizeof(struct ramfs_fs_info), GFP_KERNEL);
	sb->s_fs_info = fsi;
	if (!fsi)
		return -ENOMEM;

	err = ramfs_parse_options_bs(data, &fsi->mount_opts);
	if (err)
		return err;

	sb->s_maxbytes		= MAX_LFS_FILESIZE;
	sb->s_blocksize		= PAGE_SIZE;
	sb->s_blocksize_bits	= PAGE_SHIFT;
	sb->s_magic		= RAMFS_MAGIC_BS;
	sb->s_op		= &ramfs_ops_bs;
	sb->s_time_gran		= 1;

	inode = ramfs_get_inode_bs(sb, NULL, 
					S_IFDIR | fsi->mount_opts.mode, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

struct dentry *ramfs_mount_bs(struct file_system_type *fs_type,
			int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, ramfs_fill_super_bs);
}

static void ramfs_kill_sb_bs(struct super_block *sb)
{
	kfree(sb->s_fs_info);
	kill_litter_super(sb);
}

static struct file_system_type ramfs_fs_type_bs = {
	.name		= "ramfs_bs",
	.mount		= ramfs_mount_bs,
	.kill_sb	= ramfs_kill_sb_bs,
	.fs_flags	= FS_USERNS_MOUNT,
};

int __init init_ramfs_fs_bs(void)
{
	static unsigned long once;

	if (test_and_set_bit(0, &once))
		return 0;

	return register_filesystem(&ramfs_fs_type_bs);
}
fs_initcall(init_ramfs_fs_bs);
