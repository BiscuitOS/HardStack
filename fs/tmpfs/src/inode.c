/*
 * Tmpfs/shmem filesytem -- inode
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
#include <linux/posix_acl.h>
#include <linux/xattr.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/shmem_fs.h>
#include <linux/security.h>
#include <linux/migrate.h>

#include "internal.h"

static int shmem_reserve_inode_bs(struct super_block *sb)
{
	struct shmem_sb_info *sbinfo = SHMEM_SB_BS(sb);

	if (sbinfo->max_inodes) {
		spin_lock(&sbinfo->stat_lock);
		if (!sbinfo->free_inodes) {
			spin_unlock(&sbinfo->stat_lock);
			return -ENOSPC;
		}
		sbinfo->free_inodes--;
		spin_unlock(&sbinfo->stat_lock);
	}
	return 0;
}

static void shmem_free_inode_bs(struct super_block *sb)
{
	BS_DUP();
}

static int shmem_mknod_bs(struct inode *dir, struct dentry *dentry,
                                umode_t mode, dev_t dev);

static int shmem_create_bs(struct inode *dir, struct dentry *dentry,
					umode_t mode, bool excl)
{
	return shmem_mknod_bs(dir, dentry, mode | S_IFREG, 0);
}

/*
 * Link a file..
 */
static int shmem_link_bs(struct dentry *old_dentry, struct inode *dir,
				struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int shmem_unlink_bs(struct inode *dir, struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int shmem_symlink_bs(struct inode *dir, struct dentry *dentry,
					const char *symname)
{
	BS_DUP();
	return 0;
}

/*
 * File creation. Allocate an inode, and we're done..
 */
static int shmem_mknod_bs(struct inode *dir, struct dentry *dentry,
				umode_t mode, dev_t dev)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = shmem_get_inode_bs(dir->i_sb, dir, mode, dev, VM_NORESERVE);
	if (inode) {
		error = simple_acl_create(dir, inode);
		if (error)
			goto out_iput;

		error = security_inode_init_security(inode, dir,
				&dentry->d_name, shmem_initxattrs_bs, NULL);
		if (error && error != -EOPNOTSUPP)
			goto out_iput;

		error = 0;
		dir->i_size += BOGO_DIRENT_SIZE;
		dir->i_ctime = dir->i_mtime = current_time(dir);
		d_instantiate(dentry, inode);
		dget(dentry);	/* Extra count - pin the dentry in core */
	}
	return error;
out_iput:
	iput(inode);
	return error;
}

static int shmem_mkdir_bs(struct inode *dir, struct dentry *dentry,
							umode_t mode)
{
	int error;

	if ((error = shmem_mknod_bs(dir, dentry, mode | S_IFDIR, 0)))
		return error;
	inc_nlink(dir);
	return 0;
}

static int shmem_rmdir_bs(struct inode *dir, struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int shmem_rename2_bs(struct inode *old_dir, struct dentry *old_dentry,
	struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	BS_DUP();
	return 0;
}

static int shmem_tmpfile_bs(struct inode *dir, struct dentry *dentry,
								umode_t mode)
{
	BS_DUP();
	return 0;
}

static int shmem_writepage_bs(struct page *page, struct writeback_control *wbc)
{
	BS_DUP();
	return 0;
}

static int shmem_getpage_gfp_bs(struct inode *inode, pgoff_t index,
		struct page **page, enum sgp_type sgp, gfp_t gfp,
		struct vm_area_struct *vma, struct vm_fault *vmf,
		vm_fault_t *fault_type)
{
	BS_DUP();
	return 0;
}

int shmem_getpage_bs(struct inode *inode, pgoff_t index,
			struct page **pagep, enum sgp_type sgp)
{
	return shmem_getpage_gfp_bs(inode, index, pagep, sgp,
			mapping_gfp_mask(inode->i_mapping), NULL, NULL, NULL);
}

static int
shmem_write_begin_bs(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, unsigned flags,
		struct page **pagep, void **fsdata)
{
	struct inode *inode = mapping->host;
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	pgoff_t index = pos >> PAGE_SHIFT;

	/* i_mutex is held by caller */
	if (unlikely(info->seals & (F_SEAL_WRITE | F_SEAL_GROW))) {
		if (info->seals & F_SEAL_WRITE)
			return -EPERM;
		if ((info->seals & F_SEAL_GROW) && pos + len > inode->i_size)
			return -EPERM;
	}

	return shmem_getpage_bs(inode, index, pagep, SGP_WRITE);
}

static int
shmem_write_end_bs(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, unsigned copied,
		struct page *page, void *fsdata)
{
	BS_DUP();
	return 0;
}

static int shmem_getattr_bs(const struct path *path, struct kstat *stat,
		u32 request_mask, unsigned int query_flags)
{
	BS_DUP();
	return 0;
}

static int shmem_setattr_bs(struct dentry *dentry, struct iattr *attr)
{
	BS_DUP();
	return 0;
}

static int shmem_mmap_bs(struct file *file, struct vm_area_struct *vma)
{
	BS_DUP();
	return 0;
}

unsigned long shmem_get_unmapped_area_bs(struct file *file,
		unsigned long uaddr, unsigned long len,
		unsigned long pgoff, unsigned long flags)
{
	BS_DUP();
	return 0;
}

static loff_t shmem_file_llseek_bs(struct file *file, 
					loff_t offset, int whence)
{
	BS_DUP();
	return 0;
}

static ssize_t shmem_file_read_iter_bs(struct kiocb *kiocb, 
						struct iov_iter *to)
{
	BS_DUP();
	return 0;
}

static long shmem_fallocate_bs(struct file *file, int mode, loff_t offset,
							loff_t len)
{
	BS_DUP();
	return 0;
}

static struct mempolicy *shmem_get_sbmpol_bs(struct shmem_sb_info *sbinfo)
{
	struct mempolicy *mpol = NULL;

	if (sbinfo->mpol) {
		spin_lock(&sbinfo->stat_lock); /* prevent replace/use races */
		mpol = sbinfo->mpol;
		mpol_get(mpol);
		spin_unlock(&sbinfo->stat_lock);
	}
	return mpol;
}

static const struct inode_operations shmem_dir_inode_operations_bs = {
	.create		= shmem_create_bs,
	.lookup		= simple_lookup,
	.link		= shmem_link_bs,
	.unlink		= shmem_unlink_bs,
	.symlink	= shmem_symlink_bs,
	.mkdir		= shmem_mkdir_bs,
	.rmdir		= shmem_rmdir_bs,
	.mknod		= shmem_mknod_bs,
	.rename		= shmem_rename2_bs,
	.tmpfile	= shmem_tmpfile_bs,
};

static const struct address_space_operations shmem_aops_bs = {
	.writepage	= shmem_writepage_bs,
	.set_page_dirty	= __set_page_dirty_no_writeback,
	.write_begin	= shmem_write_begin_bs,
	.write_end	= shmem_write_end_bs,
	.migratepage	= migrate_page,
	.error_remove_page = generic_error_remove_page,
};

static const struct inode_operations shmem_inode_operations_bs = {
	.getattr	= shmem_getattr_bs,
	.setattr	= shmem_setattr_bs,
};

static const struct file_operations shmem_file_operations_bs = {
	.mmap		= shmem_mmap_bs,
	.get_unmapped_area = shmem_get_unmapped_area_bs,
	.llseek		= shmem_file_llseek_bs,
	.read_iter	= shmem_file_read_iter_bs,
	.write_iter	= generic_file_write_iter,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.fallocate	= shmem_fallocate_bs,
};

struct inode *shmem_get_inode_bs(struct super_block *sb, 
	const struct inode *dir, umode_t mode, dev_t dev, unsigned long flags)
{
	struct inode *inode;
	struct shmem_inode_info *info;
	struct shmem_sb_info *sbinfo = SHMEM_SB_BS(sb);

	if (shmem_reserve_inode_bs(sb))
		return NULL;

	inode = new_inode(sb);
	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_blocks = 0;
		inode->i_atime = inode->i_mtime =
				inode->i_ctime = current_time(inode);
		inode->i_generation = prandom_u32();
		info = SHMEM_I_BS(inode);
		memset(info, 0, (char *)inode - (char *)info);
		spin_lock_init(&info->lock);
		info->seals = F_SEAL_SEAL;
		info->flags = flags & VM_NORESERVE;
		INIT_LIST_HEAD(&info->shrinklist);
		INIT_LIST_HEAD(&info->swaplist);
		simple_xattrs_init(&info->xattrs);
		cache_no_acl(inode);

		switch (mode & S_IFMT) {
		default:
			BS_DUP();
			break;
		case S_IFREG:
			inode->i_mapping->a_ops = &shmem_aops_bs;
			inode->i_op = &shmem_inode_operations_bs;
			inode->i_fop = &shmem_file_operations_bs;
			mpol_shared_policy_init(&info->policy,
					shmem_get_sbmpol_bs(sbinfo));
			break;
		case S_IFDIR:
			inc_nlink(inode);
			/* Some things misbehave if size == 0 on a directory */
			inode->i_size = 2 * BOGO_DIRENT_SIZE;
			inode->i_op = &shmem_dir_inode_operations_bs;
			inode->i_fop = &simple_dir_operations;
			break;
		case S_IFLNK:
			BS_DUP();
			break;
		}

		lockdep_annotate_inode_mutex_key(inode);
	} else
		shmem_free_inode_bs(sb);
	return inode;
}
