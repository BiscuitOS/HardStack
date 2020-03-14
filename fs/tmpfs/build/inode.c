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
#include <linux/mm_types.h>
#include <linux/swapops.h>
#include <linux/userfaultfd_k.h>
#include <linux/mman.h>
#include <linux/memcontrol.h>
#include <linux/highmem.h>
#include <linux/uio.h>

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
static const struct address_space_operations shmem_aops_bs;

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

/*
 * ... whereas tmpfs objects are accounted incrementally as
 * pages are allocated, in order ot allow large sparse files.
 * shmem_getpage reports shmem_acct_block failure as -ENOSPC not -ENOMEM,
 * so that a failure on a sparse tmpfs mapping will give SIGBUG not OOM.
 */
static inline int shmem_acct_block_bs(unsigned long flags, long pages)
{
	if (!(flags & VM_NORESERVE))
		return 0;

	return security_vm_enough_memory_mm(current->mm, 
					pages * VM_ACCT(PAGE_SIZE));
}

static inline void shmem_unacct_blocks_bs(unsigned long flags, long pages)
{
	if (flags & VM_NORESERVE)
		vm_unacct_memory(pages * VM_ACCT(PAGE_SIZE));
}

static inline bool shmem_inode_acct_block_bs(struct inode *inode, long pages)
{
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	struct shmem_sb_info *sbinfo = SHMEM_SB_BS(inode->i_sb);

	if (shmem_acct_block_bs(info->flags, pages))
		return false;

	if (sbinfo->max_blocks) {
		if (percpu_counter_compare(&sbinfo->used_blocks,
				sbinfo->max_blocks - pages) > 0)
			goto unacct;
		percpu_counter_add(&sbinfo->used_blocks, pages);
	}

	return true;
unacct:
	shmem_unacct_blocks_bs(info->flags, pages);
	return false;
}

static struct page *shmem_alloc_hugepage_bs(gfp_t gfp,
		struct shmem_inode_info *info, pgoff_t index)
{
	BS_DUP();
	return NULL;
}

static void shmem_pseudo_vma_init_bs(struct vm_area_struct *vma,
		struct shmem_inode_info *info, pgoff_t index)
{
	/* Create a pseudo vma that just contains the policy */
	vma_init(vma, NULL);
	/* Bias interleave by inode number to distribute better across nodes */
	vma->vm_pgoff = index + info->vfs_inode.i_ino;
	vma->vm_policy = mpol_shared_policy_lookup(&info->policy, index);
}

static void shmem_pseudo_vma_destroy_bs(struct vm_area_struct *vma)
{
	/* Drop reference taken by mpol_shared_policy_lookup() */
	mpol_cond_put(vma->vm_policy);
}

static struct page *shmem_alloc_page_bs(gfp_t gfp,
		struct shmem_inode_info *info, pgoff_t index)
{
	struct vm_area_struct pvma;
	struct page *page;

	shmem_pseudo_vma_init_bs(&pvma, info, index);
	page = alloc_page_vma(gfp, &pvma, 0);
	shmem_pseudo_vma_destroy_bs(&pvma);

	return page;
}

static inline void shmem_inode_unacct_blocks_bs(struct inode *inode,
							long pages)
{
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	struct shmem_sb_info *sbinfo = SHMEM_SB_BS(inode->i_sb);

	if (sbinfo->max_blocks)
		percpu_counter_sub(&sbinfo->used_blocks, pages);
	shmem_unacct_blocks_bs(info->flags, pages);
}

static struct page *shmem_alloc_and_acct_page_bs(gfp_t gfp,
			struct inode *inode, pgoff_t index, bool huge)
{
	struct shmem_inode_info *info = SHMEM_I(inode);
	struct page *page;
	int nr;
	int err = -ENOSPC;

	if (!IS_ENABLED(CONFIG_TRANSPARENT_HUGE_PAGECACHE))
		huge = false;
	nr = huge ? HPAGE_PMD_NR : 1;

	if (!shmem_inode_acct_block_bs(inode, nr))
		goto failed;

	if (huge)
		page = shmem_alloc_hugepage_bs(gfp, info, index);
	else
		page = shmem_alloc_page_bs(gfp, info, index);
	if (page) {
		__SetPageLocked(page);
		__SetPageSwapBacked(page);
		return page;
	}

	err = -ENOMEM;
	shmem_inode_unacct_blocks_bs(inode, nr);

failed:
	return ERR_PTR(err);
}

/*
 * shmem_recalc_indoe - recalculate the block usage of an inode
 */
static void shmem_recalc_inode_bs(struct inode *inode)
{
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	long freed;

	freed = info->alloced - info->swapped - inode->i_mapping->nrpages;
	if (freed > 0) {
		info->alloced -= freed;
		inode->i_blocks -= freed * BLOCKS_PER_PAGE;
		shmem_inode_unacct_blocks_bs(inode, freed);
	}
}

static int shmem_getpage_gfp_bs(struct inode *inode, pgoff_t index,
		struct page **pagep, enum sgp_type sgp, gfp_t gfp,
		struct vm_area_struct *vma, struct vm_fault *vmf,
		vm_fault_t *fault_type)
{
	struct address_space *mapping = inode->i_mapping;
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	struct shmem_sb_info *sbinfo;
	struct mm_struct *charge_mm;
	struct mem_cgroup *memcg;
	struct page *page;
	swp_entry_t swap;
	enum sgp_type sgp_huge = sgp;
	pgoff_t hindex = index;
	int error;
	int alloced = 0;

	if (index > (MAX_LFS_FILESIZE >> PAGE_SHIFT))
		return -EFBIG;
	if (sgp == SGP_NOHUGE || sgp == SGP_HUGE)
		sgp = SGP_CACHE;

repeat:
	swap.val = 0;
	page = find_lock_entry(mapping, index);
	if (xa_is_value(page)) {
		swap = radix_to_swp_entry(page);
		page = NULL;
	}

	if (sgp <= SGP_CACHE &&
		((loff_t)index << PAGE_SHIFT) >= i_size_read(inode)) {
		error = -EINVAL;
		goto unlock;
	}

	if (page && sgp == SGP_WRITE)
		mark_page_accessed(page);

	/* fallocated page? */
	if (page && !PageUptodate(page)) {
		if (sgp != SGP_READ)
			goto clear;
		unlock_page(page);
		put_page(page);
		page = NULL;
	}

	if (page || (sgp == SGP_READ && !swap.val)) {
		*pagep = page;
		return 0;
	}

	/*
	 * Fast cache lookup did not find it:
	 * bring it back from swap or allocate.
	 */
	sbinfo = SHMEM_SB_BS(inode->i_sb);
	charge_mm = vma ? vma->vm_mm : current->mm;

	if (swap.val) {
		BS_DUP();
	} else {
		if (vma && userfaultfd_missing(vma)) {
			*fault_type = handle_userfault(vmf, VM_UFFD_MISSING);
			return 0;
		}

		/* shmem_symlink_bs() */
		if (mapping->a_ops != &shmem_aops_bs)
			goto alloc_nohuge;
		if (shmem_huge == SHMEM_HUGE_DENY || sgp_huge == SGP_NOHUGE)
			goto alloc_nohuge;
		if (shmem_huge == SHMEM_HUGE_FORCE)
			goto alloc_huge;
		switch (sbinfo->huge) {
			loff_t i_size;
			pgoff_t off;
		case SHMEM_HUGE_NEVER:
			goto alloc_nohuge;
		case SHMEM_HUGE_WITHIN_SIZE:
			off = round_up(index, HPAGE_PMD_NR);
			i_size = round_up(i_size_read(inode), PAGE_SIZE);
			if (i_size >= HPAGE_PMD_SIZE &&
				i_size >> PAGE_SHIFT >= off)
				goto alloc_huge;
			/* fallthrough */
		case SHMEM_HUGE_ADVISE:
			if (sgp_huge == SGP_HUGE)
				goto alloc_huge;
			/* TODO: implement fadvise() hints */
			goto alloc_nohuge;
		}
alloc_huge:
		page = shmem_alloc_and_acct_page_bs(gfp, inode, index, true);
		if (IS_ERR(page)) {
alloc_nohuge:
			page = shmem_alloc_and_acct_page_bs(gfp, inode,
								index, false);
		}
		if (IS_ERR(page)) {
			BS_DUP();
		}

		if (PageTransHuge(page))
			hindex = round_down(index, HPAGE_PMD_NR);
		else
			hindex = index;

		if (sgp == SGP_WRITE)
			__SetPageReferenced(page);

		error = mem_cgroup_try_charge_delay(page, charge_mm, gfp,
				&memcg, PageTransHuge(page));
		if (error) {
			mem_cgroup_cancel_charge(page, memcg, 
							PageTransHuge(page));
			goto unacct;
		}
		mem_cgroup_commit_charge(page, memcg, false,
							PageTransHuge(page));
		lru_cache_add_anon(page);

		spin_lock_irq(&info->lock);
		info->alloced += 1 << compound_order(page);
		inode->i_blocks += BLOCKS_PER_PAGE << compound_order(page);
		shmem_recalc_inode_bs(inode);
		spin_unlock_irq(&info->lock);
		alloced = true;

		if (PageTransHuge(page) &&
			DIV_ROUND_UP(i_size_read(inode), PAGE_SIZE) <
				hindex + HPAGE_PMD_NR - 1) {
			BS_DUP();
		}

		/*
		 * Let SGP_FALLOC use the SGP_WRITE optimization on a new page.
		 */
		if (sgp == SGP_FALLOC)
			sgp = SGP_WRITE;
clear:
		/*
		 * Let SGP_WRITE caller clear ends if write does not fill page;
		 * but SGP_FALLOC on a page fallocated earlier must initialize
		 * it now, lest undo on failure cancel our earlier guarantee.
		 */
		if (sgp != SGP_WRITE && !PageUptodate(page)) {
			struct page *head = compound_head(page);
			int i;

			for (i = 0; i < (1 << compound_order(head)); i++) {
				clear_highpage(head + i);
				flush_dcache_page(head + i);
			}
			SetPageUptodate(head);
		}
	}

	/* Perhaps the file has been truncated since we checked */
	if (sgp <= SGP_CACHE &&
		((loff_t)index << PAGE_SHIFT) >= i_size_read(inode)) {
		BS_DUP();
	}
	*pagep = page + index - hindex;
	return 0;

	/* Error recovery */
unacct:
	BS_DUP();
unlock:
	BS_DUP();
	return error;
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

	/* SGP_WRITE: may exceed i_size, may allocate !Uptodate page */
	return shmem_getpage_bs(inode, index, pagep, SGP_WRITE);
}

static int
shmem_write_end_bs(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, unsigned copied,
		struct page *page, void *fsdata)
{
	struct inode *inode = mapping->host;

	if (pos + copied > inode->i_size)
		i_size_write(inode, pos + copied);

	if (!PageUptodate(page)) {
		struct page *head = compound_head(page);

		if (PageTransCompound(page)) {
			BS_DUP();
		}
		if (copied < PAGE_SIZE) {
			unsigned from = pos & (PAGE_SIZE - 1);

			zero_user_segments(page, 0, from,
						from + copied, PAGE_SIZE);
		}
		SetPageUptodate(head);
	}
	set_page_dirty(page);
	unlock_page(page);
	put_page(page);

	return copied;
}

static inline bool is_huge_enabled_bs(struct shmem_sb_info *sbinfo)
{
	if (IS_ENABLED(CONFIG_TRANSPARENT_HUGE_PAGECACHE) &&
		(shmem_huge == SHMEM_HUGE_FORCE || sbinfo->huge) &&
		 shmem_huge != SHMEM_HUGE_DENY)
		return true;
	return false;
}

static int shmem_getattr_bs(const struct path *path, struct kstat *stat,
		u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = path->dentry->d_inode;
	struct shmem_inode_info *info = SHMEM_I_BS(inode);
	struct shmem_sb_info *sb_info = SHMEM_SB_BS(inode->i_sb);

	if (info->alloced - info->swapped != inode->i_mapping->nrpages) {
		BS_DUP();
	}
	generic_fillattr(inode, stat);

	if (is_huge_enabled_bs(sb_info))
		stat->blksize = HPAGE_PMD_SIZE;

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

static ssize_t shmem_file_read_iter_bs(struct kiocb *iocb, 
						struct iov_iter *to)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file_inode(file);
	struct address_space *mapping = inode->i_mapping;
	pgoff_t index;
	unsigned long offset;
	enum sgp_type sgp = SGP_READ;
	int error = 0;
	ssize_t retval = 0;
	loff_t *ppos = &iocb->ki_pos;

	/*
	 * Might this read be for a stacking filesystem? Then when reading
	 * holes of a sparse file, we actually need to allocate those pages,
	 * and even mark them dirty, so it cannot exceed the max_blocks limit.
	 */
	if (!iter_is_iovec(to))
		sgp = SGP_CACHE;

	index = *ppos >> PAGE_SHIFT;
	offset = *ppos & ~PAGE_MASK;

	for (;;) {
		struct page *page = NULL;
		pgoff_t end_index;
		unsigned long nr, ret;
		loff_t i_size = i_size_read(inode);

		end_index = i_size >> PAGE_SHIFT;
		if (index > end_index)
			break;
		if (index == end_index) {
			nr = i_size & ~PAGE_MASK;
			if (nr <= offset)
				break;
		}

		error = shmem_getpage_bs(inode, index, &page, sgp);
		if (error) {
			if (error == -EINVAL)
				error = 0;
			break;
		}
		if (page) {
			if (sgp == SGP_CACHE)
				set_page_dirty(page);
			unlock_page(page);
		}

		/*
		 * We must evaluate after, since reads (unlike writes)
		 * are callled without i_mutex protection against truncate
		 */
		nr = PAGE_SIZE;
		i_size = i_size_read(inode);
		end_index = i_size >> PAGE_SHIFT;
		if (index == end_index) {
			nr = i_size & ~PAGE_MASK;
			if (nr <= offset) {
				if (page)
					put_page(page);
				break;
			}
		}
		nr -= offset;

		if (page) {
			/*
			 * If users can be writing to this page using arbitrary
			 * virtual addresses, take care about potential aliasing
			 * before reading the apge on the kernel side.
			 */
			if (mapping_writably_mapped(mapping))
				flush_dcache_page(page);

			/*
			 * Mark the page accessed if we read the beginning.
			 */
			if (!offset)
				mark_page_accessed(page);
		} else {
			page = ZERO_PAGE(0);
			get_page(page);
		}

		/*
		 * Ok, we have the page, and it's up-to- date, so
		 * now we can copy it to user space...
		 */
		ret = copy_page_to_iter(page, offset, nr, to);
		retval += ret;
		offset += ret;
		index += offset >> PAGE_SHIFT;
		offset &= ~PAGE_MASK;

		put_page(page);
		if (!iov_iter_count(to))
			break;
		if (ret < nr) {
			error = -EFAULT;
			break;
		}
		cond_resched();
	}

	*ppos = ((loff_t) index << PAGE_SHIFT) + offset;
	file_accessed(file);
	return retval ? retval : error;

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
