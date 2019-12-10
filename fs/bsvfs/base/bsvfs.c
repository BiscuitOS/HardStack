/*
 * BiscuitOS filesystem
 *
 * (C) 2019.12.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/xattr.h>
#include <linux/random.h>
#include <linux/migrate.h>
#include <linux/exportfs.h>
#include <linux/mempolicy.h>
#include <linux/posix_acl.h>
#include <linux/security.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/uio.h>
#include <linux/swapops.h>
#include <linux/userfaultfd_k.h>

/* fs maigc */
#define BISCUITOS_FS_MAGIC	0x911016
#define BLOCKS_PER_PAGE		(PAGE_SIZE/512)

/* Pretend that each entry is of this size in directory's i_size */
#define BOGO_DIRENT_SIZE	20
#define VM_ACCT(size)		(PAGE_ALIGN(size) >> PAGE_SHIFT)
/* Not support xattr */
#define BiscuitOS_initxattrs	NULL
/* Symlink up to this size is knalloc'ed instead of using a swappable page */
#define SHORT_SYMLINK_LEN	128

/*
 * The set of flags that only affect watermark checking and reclaim
 * behaviour. This is used by the MM to obey the caller constraints
 * about IO, FS and watermark checking while ignoring placement
 * hints such as HIGHMEM usage.
 */
#define GFP_RECLAIM_MASK (__GFP_RECLAIM|__GFP_HIGH|__GFP_IO|__GFP_FS|\
			__GFP_NOWARN|__GFP_RETRY_MAYFAIL|__GFP_NOFAIL|\
			__GFP_NORETRY|__GFP_MEMALLOC|__GFP_NOMEMALLOC|\
			__GFP_ATOMIC)

/*
 * Definitions for "huge tmpfs": tmpfs mounted with the huge= option
 *
 * BISCUITOS_HUGE_NEVER:
 *      disables huge pages for the mount;
 * BISCUITOS_HUGE_ALWAYS:
 *      enables huge pages for the mount;
 * BISCUITOS_HUGE_WITHIN_SIZE:
 *      only allocate huge pages if the page will be fully within i_size,
 *      also respect fadvise()/madvise() hints;
 * BISCUITOS_HUGE_ADVISE:
 *      only allocate huge pages if requested with fadvise()/madvise();
 */
#define BISCUITOS_HUGE_NEVER		0
#define BISCUITOS_HUGE_ALWAYS		1
#define BISCUITOS_HUGE_WITHIN_SIZE	2
#define BISCUITOS_HUGE_ADVISE		3

/*
 * Special values.
 * Only can be set via /sys/kernel/mm/transparent_hugepage/shmem_enabled:
 *
 * BISCUITOS_HUGE_DENY:
 *      disables huge on shm_mnt and all mounts, for emergency use;
 * BISCUITOS_HUGE_FORCE:
 *      enables huge on shm_mnt and all mounts, w/o needing option, for testing;
 *
 */
#define BISCUITOS_HUGE_DENY		(-1)
#define BISCUITOS_HUGE_FORCE		(-2)

#define BiscuitOS_huge	BISCUITOS_HUGE_DENY

/* BiscuitOS inode info */
struct BiscuitOS_inode_info
{
	spinlock_t		lock;
	unsigned int		seals;		/* BiscuitOS seals */
	unsigned long		flags;
	unsigned long		alloced;	/* data pages alloced to file */
	unsigned long		swapped;	/* subtotal assigned to swap */
	struct list_head	shrinklist;	/* shrinkable hpage inodes */
	struct list_head	swaplist;	/* chain of maybes on swap */
	struct shared_policy	policy;		/* NUMA memory alloc policy */
	struct simple_xattrs	xattrs;		/* list of xattrs */
	struct inode		vfs_inode;
};

/* BiscuitOS sb info */
struct BiscuitOS_sb_info
{
	unsigned long max_blocks;     /* how many blocks are allowed */
	struct percpu_counter used_blocks; /* How many are allocated */
	unsigned long max_inodes;     /* how many inode are allowed */
	unsigned long free_inodes;    /* how many are left for allocation */
	spinlock_t stat_lock;         /* Serialize BiscuitOS_sb_info changes */
	umode_t mode;                 /* Mount mode for root directory */
	unsigned char huge;           /* Whether to try for hugepages */
	kuid_t uid;                   /* Mount uid for root directory */
	kgid_t gid;                   /* Mount gid for root directory */
	struct mempolicy *mpol;       /* default memory policy for mappings */
	spinlock_t shrinklist_lock;   /* Protects shrinklist */
	struct list_head shrinklist;  /* List of shrinkable inodes */
	unsigned long shrinklist_len; /* Length of shrinklist */
};

/* Flag allocation requirements to BiscuitOS_getpage */
enum bgp_type {
	BGP_READ,	/* don't exceed i_size, don't allocate page */
	BGP_CACHE,	/* don't exceed i_size, may allocate page */
	BGP_NOHUGE,	/* like BGP_CACHE, but no huge pages */
	BGP_HUGE,	/* like BGP_CACHE, huge pages preferred */
	BGP_WRITE,	/* may exceed i_size, may allocate !Uptodate page */
	BGP_FALLOC,	/* like BGP_WRITE, but make existing page Uptodate */
};

/* inode cache */
static struct kmem_cache *BiscuitOS_inode_cachep;
/* mount pointer */
static struct vfsmount *BiscuitOS_mnt;

static LIST_HEAD(BiscuitOS_swaplist);
static DEFINE_MUTEX(BiscuitOS_swaplist_mutex);

/* head list */
static const struct inode_operations BiscuitOS_special_inode_operations;
static const struct address_space_operations BiscuitOS_aops;
static const struct inode_operations BiscuitOS_inode_operations;
static const struct file_operations BiscuitOS_file_operations;
static const struct inode_operations BiscuitOS_dir_inode_operations;
static const struct super_operations BiscuitOS_ops;
static const struct export_operations BiscuitOS_export_ops;
static const struct inode_operations BiscuitOS_short_symlink_operations;
static const struct inode_operations BiscuitOS_symlink_inode_operations;
static void BiscuitOS_recalc_inode(struct inode *inode);
static struct inode *BiscuitOS_get_inode(struct super_block *,
			const struct inode *, umode_t, dev_t, unsigned long);
static void BiscuitOS_free_inode(struct super_block *sb);

static void BiscuitOS_init_inode(void *foo)
{
	struct BiscuitOS_inode_info *info = foo;
	inode_init_once(&info->vfs_inode);
}

static void BiscuitOS_init_inodecache(void)
{
	BiscuitOS_inode_cachep = kmem_cache_create("BiscuitOS_inode_cache",
					sizeof(struct BiscuitOS_inode_info),
					0,
					SLAB_PANIC | SLAB_ACCOUNT,
					BiscuitOS_init_inode);
}

static void BiscuitOS_destroy_inodecache(void)
{
	kmem_cache_destroy(BiscuitOS_inode_cachep);
}

static unsigned long BiscuitOS_default_max_blocks(void)
{
	return totalram_pages() / 10;
}

static unsigned long BiscuitOS_default_max_inodes(void)
{
	return totalram_pages() / 10;
}

static struct dentry *BiscuitOS_get_parent(struct dentry *child)
{
	return ERR_PTR(-ESTALE);
}

static inline struct BiscuitOS_inode_info *BISCUITOS_I(struct inode *inode)
{
	return container_of(inode, struct BiscuitOS_inode_info, vfs_inode);
}

static inline struct BiscuitOS_sb_info *BISCUITOS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline void BiscuitOS_unacct_size(unsigned long flags, loff_t size)
{
	if (!(flags & VM_NORESERVE))
		vm_unacct_memory(VM_ACCT(size));
}

static void BiscuitOS_undo_range(struct inode *inode, 
				loff_t lstart, loff_t lend, bool unfalloc)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
}

static void BiscuitOS_truncate_range(struct inode *inode, 
						loff_t lstart, loff_t lend)
{
	BiscuitOS_undo_range(inode, lstart, lend, false);
	inode->i_ctime = inode->i_mtime = current_time(inode);
}

static int BiscuitOS_encode_fh(struct inode *inode, __u32 *fh,
			int *len, struct inode *parent)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static struct dentry *BiscuitOS_fh_to_dentry(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return NULL;
}

static void BiscuitOS_put_super(struct super_block *sb)
{
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(sb);

	percpu_counter_destroy(&sbinfo->used_blocks);
	mpol_put(sbinfo->mpol);
	kfree(sbinfo);
	sb->s_fs_info = NULL;
}

static struct inode *BiscuitOS_alloc_inode(struct super_block *sb)
{
	struct BiscuitOS_inode_info *info;
	info = kmem_cache_alloc(BiscuitOS_inode_cachep, GFP_KERNEL);
	if (!info)
		return NULL;
	return &info->vfs_inode;
}

static void BiscuitOS_destroy_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);
	if (S_ISLNK(inode->i_mode))
		kfree(inode->i_link);
	kmem_cache_free(BiscuitOS_inode_cachep, BISCUITOS_I(inode));
}

static void BiscuitOS_destroy_inode(struct inode *inode)
{
	if (S_ISREG(inode->i_mode))
		mpol_free_shared_policy(&BISCUITOS_I(inode)->policy);
	call_rcu(&inode->i_rcu, BiscuitOS_destroy_callback);
}

static int BiscuitOS_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_remount_fs(struct super_block *sb, int *flags, char *data)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_show_options(struct seq_file *seq, struct dentry *root)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static void BiscuitOS_evict_inode(struct inode *inode)
{
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(inode->i_sb);

	if (inode->i_mapping->a_ops == &BiscuitOS_aops) {
		BiscuitOS_unacct_size(info->flags, inode->i_size);
		inode->i_size = 0;
		BiscuitOS_truncate_range(inode, 0, (loff_t)-1);
		if (!list_empty(&info->shrinklist)) {
			spin_lock(&sbinfo->shrinklist_lock);
			if (!list_empty(&info->shrinklist)) {
				list_del_init(&info->shrinklist);
				sbinfo->shrinklist_len--;
			}
			spin_unlock(&sbinfo->shrinklist_lock);
		}
		if (!list_empty(&info->swaplist)) {
			mutex_lock(&BiscuitOS_swaplist_mutex);
			list_del_init(&info->swaplist);
			mutex_unlock(&BiscuitOS_swaplist_mutex);
		}
	}

	simple_xattrs_free(&info->xattrs);
	WARN_ON(inode->i_blocks);
	BiscuitOS_free_inode(inode->i_sb);
	clear_inode(inode);
}

static inline bool is_huge_enabled(struct BiscuitOS_sb_info *sbinfo)
{
	if (IS_ENABLED(CONFIG_TRANSPARENT_HUGE_PAGECACHE) &&
		(BiscuitOS_huge == BISCUITOS_HUGE_FORCE || sbinfo->huge) &&
			BiscuitOS_huge != BISCUITOS_HUGE_DENY)
		return true;
	return false;
}

static int BiscuitOS_getattr(const struct path *path, struct kstat *stat,
				u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = path->dentry->d_inode;
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sb_info = BISCUITOS_SB(inode->i_sb);

	if (info->alloced - info->swapped != inode->i_mapping->nrpages) {
		spin_lock_irq(&info->lock);
		BiscuitOS_recalc_inode(inode);
		spin_unlock_irq(&info->lock);
	}
	generic_fillattr(inode, stat);

	if (is_huge_enabled(sb_info))
		stat->blksize = HPAGE_PMD_SIZE;

	return 0;
}

static inline int BiscuitOS_reacct_size(unsigned long flags,
					loff_t oldsize, loff_t newsize)
{
	if (!(flags & VM_NORESERVE)) {
		if (VM_ACCT(newsize) > VM_ACCT(oldsize))
			return security_vm_enough_memory_mm(current->mm,
					VM_ACCT(newsize) - VM_ACCT(oldsize));
		else if (VM_ACCT(newsize) < VM_ACCT(oldsize))
			vm_unacct_memory(VM_ACCT(oldsize) - VM_ACCT(newsize));
	}
	return 0;
}

static int BiscuitOS_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct inode *inode = d_inode(dentry);
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(inode->i_sb);
	int error;

	error = setattr_prepare(dentry, attr);
	if (error)
		return error;

	if (S_ISREG(inode->i_mode) && (attr->ia_valid & ATTR_SIZE)) {
		loff_t oldsize = inode->i_size;
		loff_t newsize = attr->ia_size;

		/* protected by i_mutex */
		if ((newsize < oldsize && (info->seals & F_SEAL_SHRINK)) ||
			(newsize > oldsize && (info->seals & F_SEAL_GROW)))
			return -EPERM;

		if (newsize != oldsize) {
			error = BiscuitOS_reacct_size(
				BISCUITOS_I(inode)->flags, oldsize, newsize);
			if (error)
				return error;
			i_size_write(inode, newsize);
			inode->i_ctime = inode->i_mtime = current_time(inode);
		}
		if (newsize <= oldsize) {
			loff_t holebegin = round_up(newsize, PAGE_SIZE);
			if (oldsize > holebegin)
				unmap_mapping_range(inode->i_mapping,
							holebegin, 0, 1);
			if (info->alloced)
				BiscuitOS_truncate_range(inode,
							newsize, (loff_t)-1);
			/* unmap again to remove recily COWed private pages */
			if (oldsize > holebegin)
				unmap_mapping_range(inode->i_mapping,
							holebegin, 0, 1);

			/*
			 * Part of the huge page can be beyond i_size: subject
			 * to shrink under memory pressure.
			 */
			if (IS_ENABLED(CONFIG_TRANSPARENT_HUGE_PAGECACHE)) {
				spin_lock(&sbinfo->shrinklist_lock);
				/*
				 * careful to defend against unlocked access to
				 * ->shrink_list in 
				 * BiscuitOS_unused_huge_shrink()
				 */
				if (list_empty_careful(&info->shrinklist)) {
					list_add_tail(&info->shrinklist,
						&sbinfo->shrinklist);
					sbinfo->shrinklist_len++;
				}
				spin_unlock(&sbinfo->shrinklist_lock);
			}
		}
	}
	setattr_copy(inode, attr);
	if (attr->ia_valid & ATTR_MODE)
		error = posix_acl_chmod(inode, inode->i_mode);
	return error;
}

static int BiscuitOS_mmap(struct file *file, struct vm_area_struct *vma)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_reserve_inode(struct super_block *sb)
{
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(sb);
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

static void BiscuitOS_free_inode(struct super_block *sb)
{
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(sb);
	if (sbinfo->max_inodes) {
		spin_lock(&sbinfo->stat_lock);
		sbinfo->free_inodes++;
		spin_unlock(&sbinfo->stat_lock);
	}
}

static int BiscuitOS_writepage(struct page *page, 
					struct writeback_control *wbc)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static struct page *BiscuitOS_swapin(swp_entry_t swap, gfp_t gfp,
			struct BiscuitOS_inode_info *info, pgoff_t index)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return NULL;
}

static bool BiscuitOS_confirm_swap(struct address_space *mapping,
					pgoff_t index, swp_entry_t swap)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

/*
 * When a page is moved from swapcache to shmem filecache (either by the
 * usual swapin of shmem_getpage_gfp(), or by the less common swapoff of
 * shmem_unuse_inode()), it may have been read in earlier from swap, in
 * ignorance of the mapping it belongs to.  If that mapping has special
 * constraints (like the gma500 GEM driver, which requires RAM below 4GB),
 * we may need to copy to a suitable page before moving to filecache.
 *
 * In a future release, this may well be extended to respect cpuset and
 * NUMA mempolicy, and applied also to anonymous pages in do_swap_page();
 * but for now it is a simple matter of zone.
 */
static bool BiscuitOS_should_replace_page(struct page *page, gfp_t gfp)
{
	return page_zonenum(page) > gfp_zone(gfp);
}

static int BiscuitOS_replace_page(struct page **pagep, gfp_t gfp,
			struct BiscuitOS_inode_info *info, pgoff_t index)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

/* Like add_to_page_cache_locked, but error if expected item has gone */
static int BiscuitOS_add_to_page_cache(struct page *page,
			struct address_space *mapping, pgoff_t index,
			void *expected, gfp_t gfp)
{
	XA_STATE_ORDER(xas, &mapping->i_pages, index, compound_order(page));
	unsigned long i = 0;
	unsigned long nr = 1UL << compound_order(page);

	VM_BUG_ON_PAGE(PageTail(page), page);
	VM_BUG_ON_PAGE(index != round_down(index, nr), page);
	VM_BUG_ON_PAGE(!PageLocked(page), page);
	VM_BUG_ON_PAGE(!PageSwapBacked(page), page);
	VM_BUG_ON(expected && PageTransHuge(page));


	page_ref_add(page, nr);
	page->mapping = mapping;
	page->index = index;

	do {
		void *entry;
		xas_lock_irq(&xas);
		entry = xas_find_conflict(&xas);
		if (entry != expected)
			xas_set_err(&xas, -EEXIST);
		xas_create_range(&xas);
		if (xas_error(&xas))
			goto unlock;
next:
		xas_store(&xas, page + i);
		if (++i < nr) {
			xas_next(&xas);
			goto next;
		}
		if (PageTransHuge(page)) {
			count_vm_event(THP_FILE_ALLOC);
			__inc_node_page_state(page, NR_SHMEM_THPS);
		}
		mapping->nrpages += nr;
		__mod_node_page_state(page_pgdat(page), NR_FILE_PAGES, nr);
		__mod_node_page_state(page_pgdat(page), NR_SHMEM, nr);
unlock:
		xas_unlock_irq(&xas);
	} while (xas_nomem(&xas, gfp));
	
	if (xas_error(&xas)) {
		page->mapping = NULL;
		page_ref_sub(page, nr);
		return xas_error(&xas);
	}

	return 0;
}

static inline void BiscuitOS_unacct_blocks(unsigned long flags, long pages)
{
	if (flags & VM_NORESERVE)
		vm_unacct_memory(pages * VM_ACCT(PAGE_SIZE));
}

static inline void BiscuitOS_inode_unacct_blocks(struct inode *inode, 
								long pages)
{
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(inode->i_sb);

	if (sbinfo->max_blocks)
		percpu_counter_sub(&sbinfo->used_blocks, pages);
	BiscuitOS_unacct_blocks(info->flags, pages);
}

/**
 * BiscuitOS_recalc_inode - recalculate the block usage of an inode
 * @inode: inode to recalc
 *
 * We have to calculate the free blocks since the mm can drop
 * undirtied hole pages behind our back.
 *
 * But normally   info->alloced == inode->i_mapping->nrpages + info->swapped
 * So mm freed is info->alloced - (inode->i_mapping->nrpages + info->swapped)
 *
 * It has to be called with the spinlock held.
 */
static void BiscuitOS_recalc_inode(struct inode *inode)
{
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	long freed;

	freed = info->alloced - info->swapped - inode->i_mapping->nrpages;
	if (freed > 0) {
		info->alloced -= freed;
		inode->i_blocks -= freed * BLOCKS_PER_PAGE;
		BiscuitOS_inode_unacct_blocks(inode, freed);
	}
}

/*
 * ... whereas BiscuitOS-fs objects are accounted incrementally as
 * pages are allocated, in order to allow large sparse files.
 * BiscuitOS_getpage reports BiscuitOS_acct_block failure as -ENOSPC
 * not -ENOMEM, so that a failure on a sparse BiscuitOS-fs mapping
 * will give SIGBUS not OOM.
 */
static inline int BiscuitOS_acct_block(unsigned long flags, long pages)
{
	if (!(flags & VM_NORESERVE))
		return 0;

	return security_vm_enough_memory_mm(current->mm,
			pages * VM_ACCT(PAGE_SIZE));
}

static inline bool BiscuitOS_inode_acct_block(struct inode *inode, long pages)
{
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(inode->i_sb);

	if (BiscuitOS_acct_block(info->flags, pages))
		return false;

	if (sbinfo->max_blocks) {
		if (percpu_counter_compare(&sbinfo->used_blocks,
					sbinfo->max_blocks - pages) > 0)
			goto unacct;
		percpu_counter_add(&sbinfo->used_blocks, pages);
	}
	return true;

unacct:
	BiscuitOS_unacct_blocks(info->flags, pages);
	return false;
}

#define vm_policy	vm_private_data

static void BiscuitOS_pseudo_vma_init(struct vm_area_struct *vma,
		struct BiscuitOS_inode_info *info, pgoff_t index)
{
	/* Create a pseudo vma that just contains the policy */
	vma_init(vma, NULL);
	/* Bias interleave by inode number to distribute better across nodes */
	vma->vm_pgoff = index + info->vfs_inode.i_ino;
	vma->vm_policy = mpol_shared_policy_lookup(&info->policy, index);
}

static void BiscuitOS_pseudo_vma_destroy(struct vm_area_struct *vma)
{
	/* Drop reference taken by mpol_shared_policy_lookup() */
	mpol_cond_put(vma->vm_policy);
}

static struct page *BiscuitOS_alloc_page(gfp_t gfp,
			struct BiscuitOS_inode_info *info, pgoff_t index)
{
	struct vm_area_struct pvma;
	struct page *page;

	BiscuitOS_pseudo_vma_init(&pvma, info, index);
	page = alloc_page_vma(gfp, &pvma, 0);
	BiscuitOS_pseudo_vma_destroy(&pvma);

	return page;
}

static struct page *BiscuitOS_alloc_and_acct_page(gfp_t gfp,
			struct inode *inode, pgoff_t index, bool huge)
{
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct page *page;
	int nr;
	int err = -ENOSPC;

	if (!IS_ENABLED(CONFIG_TRANSPARENT_HUGE_PAGECACHE))
		huge = false;
	nr = huge ? HPAGE_PMD_NR : 1;

	if (!BiscuitOS_inode_acct_block(inode, nr))
		goto failed;

	if (huge) {
		/* allocate huge page */
		printk("\n\n\n\n%s\n\n\n", __func__);
	} else
		page = BiscuitOS_alloc_page(gfp, info, index);
	if (page) {
		__SetPageLocked(page);
		__SetPageSwapBacked(page);
		return page;
	}

	err = -ENOMEM;
	BiscuitOS_inode_unacct_blocks(inode, nr);
failed:
	return ERR_PTR(err);
}

static unsigned long 
BiscuitOS_unused_huge_shrink(struct BiscuitOS_sb_info *sbinfo,
		struct shrink_control *sc, unsigned long nr_to_split)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

/*
 * BiscuitOS_getpage_gfp - find apge in cache, or get from swap, or allocate
 *
 * If we allocate a new one we do not mark it dirty. That's up to the
 * vm. If we swap it in we mark it dirty since we also free the swap
 * entry since a page cannot live in both the swap and page cache.
 *
 * fault_mm and fault_type are only supplied by BiscuitOS_fault:
 * otherwise they are NULL.
 */
static int BiscuitOS_getpage_gfp(struct inode *inode, pgoff_t index,
	struct page **pagep, enum bgp_type bgp, gfp_t gfp,
	struct vm_area_struct *vma, struct vm_fault *vmf,
			vm_fault_t *fault_type)
{
	struct address_space *mapping = inode->i_mapping;
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	struct BiscuitOS_sb_info *sbinfo;
	struct mm_struct *charge_mm;
	struct mem_cgroup *memcg;
	struct page *page;
	swp_entry_t swap;
	enum bgp_type bgp_huge = bgp;
	pgoff_t hindex = index;
	int error;
	int once = 0;
	int alloced = 0;

	if (index > (MAX_LFS_FILESIZE >> PAGE_SHIFT))
		return -EFBIG;
	if (bgp == BGP_NOHUGE || bgp == BGP_HUGE)
		bgp = BGP_CACHE;
repeat:
	swap.val = 0;
	page = find_lock_entry(mapping, index);
	if (xa_is_value(page)) {
		swap = radix_to_swp_entry(page);
		page = NULL;
	}

	if (bgp <= BGP_CACHE &&
		((loff_t)index << PAGE_SHIFT) >= i_size_read(inode)) {
		error = -EINVAL;
		goto unlock;
	}

	if (page && bgp == BGP_WRITE)
		mark_page_accessed(page);

	/* fallocated page? */
	if (page && !PageUptodate(page)) {
		if (bgp != BGP_READ)
			goto clear;
		unlock_page(page);
		put_page(page);
		page = NULL;
	}
	if (page || (bgp == BGP_READ && !swap.val)) {
		*pagep = page;
		return 0;
	}

	/*
	 * Fast cache lookup did not find it:
	 * bring it back from swap or allocate.
	 */
	sbinfo = BISCUITOS_SB(inode->i_sb);
	charge_mm = vma ? vma->vm_mm : current->mm;

	if (swap.val) {
		/* Look it up and read it in.. */
		page = lookup_swap_cache(swap, NULL, 0);
		if (!page) {
			/* Or update major stats only when swapin successds */
			if (fault_type) {
				*fault_type |= VM_FAULT_MAJOR;
				count_vm_event(PGMAJFAULT);
				count_memcg_event_mm(charge_mm, PGMAJFAULT);
			}
			/* Here we actually start the io */
			page = BiscuitOS_swapin(swap, gfp, info, index);
			if (!page) {
				error = -ENOMEM;
				goto failed;
			}
		}

		/* We have to do this with page locked to prevent races */
		lock_page(page);
		if (!PageSwapCache(page) || page_private(page) != swap.val ||
			!BiscuitOS_confirm_swap(mapping, index, swap)) {
			error = -EEXIST;
			goto unlock;
		}
		if (!PageUptodate(page)) {
			error = -EIO;
			goto failed;
		}
		wait_on_page_writeback(page);

		if (BiscuitOS_should_replace_page(page, gfp)) {
			error = BiscuitOS_replace_page(&page, 
							gfp, info, index);
			if (error)
				goto failed;
		}

		error = mem_cgroup_try_charge_delay(page, charge_mm, gfp,
								&memcg, false);
		if (!error) {
			error = BiscuitOS_add_to_page_cache(page, mapping,
					index, swp_to_radix_entry(swap), gfp);
			/*
			 * We already confirmed swap under page lock, and make
			 * no memory allocation here, so usually no possibility
			 * of error; but free_swap_and_cache() only trylocks a
			 * page, so it is just possible that the entry has been
			 * truncated or holepunched since swap was confirmed.
			 * BiscuitOS_undo_range() will have done some of the
			 * unaccounting, now delete_from_swap_cache() will do
			 * the rest.
			 * Reset swap.val? No, leave it so "failed" goes back
			 * to "repeat": reading a hole and writing should
			 * succeed.
			 */
			if (error) {
				mem_cgroup_cancel_charge(page, memcg, false);
				delete_from_swap_cache(page);
			}
		}
		if (error)
			goto failed;

		mem_cgroup_commit_charge(page, memcg, true, false);

		spin_lock_irq(&info->lock);
		info->swapped--;
		BiscuitOS_recalc_inode(inode);
		spin_unlock_irq(&info->lock);

		if (bgp == BGP_WRITE)
			mark_page_accessed(page);

		delete_from_swap_cache(page);
		set_page_dirty(page);
		swap_free(swap);
	} else {
		if (vma && userfaultfd_missing(vma)) {
			*fault_type = handle_userfault(vmf, VM_UFFD_MISSING);
			return 0;
		}

		/* BiscuitOS_synlink() */
		if (mapping->a_ops != &BiscuitOS_aops)
			goto alloc_nohuge;
		if (BiscuitOS_huge == BISCUITOS_HUGE_DENY || 
							bgp_huge == BGP_NOHUGE)
			goto alloc_nohuge;
		if (BiscuitOS_huge == BISCUITOS_HUGE_FORCE)
			goto alloc_huge;
		switch (sbinfo->huge) {
			loff_t i_size;
			pgoff_t off;
		case BISCUITOS_HUGE_NEVER:
			goto alloc_nohuge;
		case BISCUITOS_HUGE_WITHIN_SIZE:
			off = round_up(index, HPAGE_PMD_NR);
			i_size = round_up(i_size_read(inode), PAGE_SIZE);
			if (i_size >= HPAGE_PMD_SIZE &&
					i_size >> PAGE_SHIFT >= off)
				goto alloc_huge;
			/* fallthrough */
		case BISCUITOS_HUGE_ADVISE:
			if (bgp_huge == BGP_HUGE)
				goto alloc_huge;
			/* TODO: implement fadvise() hints */
			goto alloc_nohuge;
		}
alloc_huge:
		page = BiscuitOS_alloc_and_acct_page(gfp, inode, index, true);
		if (IS_ERR(page)) {
alloc_nohuge:		page = BiscuitOS_alloc_and_acct_page(gfp, inode,
								index, false);
		}
		if (IS_ERR(page)) {
			int retry = 5;
			error = PTR_ERR(page);
			page = NULL;
			if (error != -ENOSPC)
				goto failed;

			/*
			 * Try to reclaim some space by splitting a huge page
			 * beyond i_size on the filesystem.
			 */
			while (retry--) {
				int ret;
				ret = BiscuitOS_unused_huge_shrink(sbinfo, 
								NULL, 1);
				if (ret == SHRINK_STOP)
					break;
				if (ret)
					goto alloc_nohuge;
			}
			goto failed;
		}
		if (PageTransHuge(page))
			hindex = round_down(index, HPAGE_PMD_NR);
		else
			hindex = index;

		if (bgp == BGP_WRITE)
			__SetPageReferenced(page);

		error = mem_cgroup_try_charge_delay(page, charge_mm, gfp, 
						&memcg, PageTransHuge(page));
		if (error)
			goto unacct;
		error = BiscuitOS_add_to_page_cache(page, mapping, hindex,
						NULL, gfp & GFP_RECLAIM_MASK);
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
		BiscuitOS_recalc_inode(inode);
		spin_unlock_irq(&info->lock);
		alloced = true;

		if (PageTransHuge(page) &&
				DIV_ROUND_UP(i_size_read(inode), PAGE_SIZE) <
				hindex + HPAGE_PMD_NR - 1) {
			/*
			 * Part of the huge page is beyond i_size: subject
			 * to shrink under memory pressure.
			 */
			spin_lock(&sbinfo->shrinklist_lock);
			/*
			 * careful to defend against unlocked access to
			 * ->shrink_list in shmem_unused_huge_shrink()
			 */
			if (list_empty_careful(&info->shrinklist)) {
				list_add_tail(&info->shrinklist,
						&sbinfo->shrinklist);
				sbinfo->shrinklist_len++;
			}
			spin_unlock(&sbinfo->shrinklist_lock);
		}

		/*
		 * Let BGP_FALLOC use the BGP_WRITE optimization on a new page.
		 */
		if (bgp == BGP_FALLOC)
			bgp = BGP_WRITE;
clear:
		/*
		 * Let BGP_WRITE caller clear ends if write does not fill page;
		 * but BGP_FALLOC on a page fallocated earlier must initialize
		 * it now, lest undo on failure cancel our earlier guarantee.
		 */
		if (bgp != BGP_WRITE && !PageUptodate(page)) {
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
	if (bgp <= BGP_CACHE &&
		((loff_t)index << PAGE_SHIFT) >= i_size_read(inode)) {
		if (alloced) {
			ClearPageDirty(page);
			delete_from_page_cache(page);
			spin_lock_irq(&info->lock);
			BiscuitOS_recalc_inode(inode);
			spin_unlock_irq(&info->lock);
		}
		error = -EINVAL;
		goto unlock;
	}
	*pagep = page + index - hindex;
	return 0;

	/* Error recovery */
unacct:
	BiscuitOS_inode_unacct_blocks(inode, 1 << compound_order(page));

	if (PageTransHuge(page)) {
		unlock_page(page);
		put_page(page);
		goto alloc_nohuge;
	}
failed:
	if (swap.val && !BiscuitOS_confirm_swap(mapping, index, swap))
		error = -EEXIST;
unlock:
	if (page) {
		unlock_page(page);
		put_page(page);
	}
	if (error == -ENOSPC && !once++) {
		spin_lock_irq(&info->lock);
		BiscuitOS_recalc_inode(inode);
		spin_unlock_irq(&info->lock);
		goto repeat;
	}
	if (error == -EEXIST)
		goto repeat;
	return error;
}

static int BiscuitOS_getpage(struct inode *inode, pgoff_t index,
		struct page **pagep, enum bgp_type bgp)
{
	return BiscuitOS_getpage_gfp(inode, index, pagep, bgp,
			mapping_gfp_mask(inode->i_mapping), NULL, NULL, NULL);
}

static int BiscuitOS_write_begin(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned flags, struct page **pagep, void **fsdata)
{
	struct inode *inode = mapping->host;
	struct BiscuitOS_inode_info *info = BISCUITOS_I(inode);
	pgoff_t index = pos >> PAGE_SHIFT;

	/* i_mutex is held by caller */
	if (unlikely(info->seals & (F_SEAL_WRITE | F_SEAL_GROW))) {
		if (info->seals & F_SEAL_WRITE)
			return -EPERM;
		if ((info->seals & F_SEAL_GROW) && pos + len > inode->i_size)
			return -EPERM;
	}

	return BiscuitOS_getpage(inode, index, pagep, BGP_WRITE);
}

static int BiscuitOS_write_end(struct file *file,
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned copied, struct page *page, void *fsdata)
{
	struct inode *inode = mapping->host;

	if (pos + copied > inode->i_size)
		i_size_write(inode, pos + copied);

	if (!PageUptodate(page)) {
		struct page *head = compound_head(page);
		if (PageTransCompound(page)) {
			int i;

			for (i = 0; i < HPAGE_PMD_NR; i++) {
				if (head + i == page)
					continue;
				clear_highpage(head + i);
				flush_dcache_page(head + i);
			}
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

static unsigned long BiscuitOS_get_unmapped_area(struct file *file,
				unsigned long uaddr, unsigned long len,
				unsigned long pgoff, unsigned long flags)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static loff_t BiscuitOS_file_llseek(struct file *file, 
						loff_t offset, int where)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static ssize_t BiscuitOS_file_read_iter(struct kiocb *iocb,
						struct iov_iter *to)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file_inode(file);
	struct address_space *mapping = inode->i_mapping;
	pgoff_t index;
	unsigned long offset;
	enum bgp_type bgp = BGP_READ;
	int error = 0;
	ssize_t retval = 0;
	loff_t *ppos = &iocb->ki_pos;

	/*
	 * Might this read be for a stacking filesystem? Then when reading
	 * holes of a sparse file, we actually need to allocate those pages,
	 * and even mark them dirty, so it cannot exceed the max_blocks limit.
	 */
	if (!iter_is_iovec(to))
		bgp = BGP_CACHE;

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

		error = BiscuitOS_getpage(inode, index, &page, bgp);
		if (error) {
			if (error == -EINVAL)
				error = 0;
			break;
		}
		if (page) {
			if (bgp == BGP_CACHE)
				set_page_dirty(page);
			unlock_page(page);
		}

		/*
		 * We must evaluate after, since reads (unlike writes)
		 * are called without i_mutex protection against truncate
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
			 * virtual addresses, take care about potential 
			 * aliasing before reading the page on the kernel side.
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
		 * Ok, we have the page, and it's up-to-date, so
		 * now we cqan copy it to user sapce...
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
}

static long BiscuitOS_fallocate(struct file *file, int mode, 
					loff_t offset, loff_t len)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static inline struct mempolicy *BiscuitOS_get_sbmpol(
					struct BiscuitOS_sb_info *sbinfo)
{
	return NULL;
}

/* link a file.. */
static int BiscuitOS_link(struct dentry *old_dentry, struct inode *dir,
				struct dentry *dentry)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_unlink(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode = d_inode(dentry);

	if (inode->i_nlink > 1 && !S_ISDIR(inode->i_mode))
		BiscuitOS_free_inode(inode->i_sb);

	dir->i_size -= BOGO_DIRENT_SIZE;
	inode->i_ctime = dir->i_ctime = dir->i_mtime = current_time(inode);
	drop_nlink(inode);
	dput(dentry);/* Undo the count from "create" - this does all the work */
	return 0;
}

static int BiscuitOS_symlink(struct inode *dir, struct dentry *dentry,
					const char *symname)
{
	int error;
	int len;
	struct inode *inode;
	struct page *page;

	len = strlen(symname) + 1;
	if (len > PAGE_SIZE)
		return -ENAMETOOLONG;

	inode = BiscuitOS_get_inode(dir->i_sb, dir, S_IFLNK | 0777, 0,
					VM_NORESERVE);
	if (!inode)
		return -ENOSPC;

	error = security_inode_init_security(inode, dir, &dentry->d_name,
						BiscuitOS_initxattrs, NULL);
	if (error) {
		if (error != -EOPNOTSUPP) {
			iput(inode);
			return error;
		}
		error = 0;
	}

	inode->i_size = len - 1;
	if (len <= SHORT_SYMLINK_LEN) {
		inode->i_link = kmemdup(symname, len, GFP_KERNEL);
		if (!inode->i_link) {
			iput(inode);
			return -ENOMEM;
		}
		inode->i_op = &BiscuitOS_short_symlink_operations;
	} else {
		inode_nohighmem(inode);
		error = BiscuitOS_getpage(inode, 0, &page, BGP_WRITE);
		if (error) {
			iput(inode);
			return error;
		}
		inode->i_mapping->a_ops = &BiscuitOS_aops;
		inode->i_op = &BiscuitOS_symlink_inode_operations;
		memcpy(page_address(page), symname, len);
		SetPageUptodate(page);
		set_page_dirty(page);
		unlock_page(page);
		put_page(page);
	}
	dir->i_size += BOGO_DIRENT_SIZE;
	dir->i_ctime = dir->i_mtime = current_time(dir);
	d_instantiate(dentry, inode);
	dget(dentry);
	return 0;
}

/* File creation. Allocate an inode, and we're done.. */
static int BiscuitOS_mknod(struct inode *dir, struct dentry *dentry,
					umode_t mode, dev_t dev)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = BiscuitOS_get_inode(dir->i_sb, dir, mode, dev, VM_NORESERVE);
	if (inode) {
		error = simple_acl_create(dir, inode);
		if (error)
			goto out_input;
		error = security_inode_init_security(inode, dir,
						&dentry->d_name,
						BiscuitOS_initxattrs, NULL);
		if (error && error != -EOPNOTSUPP)
			goto out_input;

		error = 0;
		dir->i_size += BOGO_DIRENT_SIZE;
		dir->i_ctime = dir->i_mtime = current_time(dir);
		d_instantiate(dentry, inode);
		dget(dentry); /* Extra count - pin the dentry in core */
	}
	return error;

out_input:
	iput(inode);
	return error;
}

static int BiscuitOS_create(struct inode *dir, struct dentry *dentry,
						umode_t mode, bool excl)
{
	return BiscuitOS_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int BiscuitOS_mkdir(struct inode *dir, 
				struct dentry *dentry, umode_t mode)
{
	int error;

	if ((error = BiscuitOS_mknod(dir, dentry, mode | S_IFDIR, 0)))
		return error;
	inc_nlink(dir);
	return 0;
}

static int BiscuitOS_rmdir(struct inode *dir, struct dentry *dentry)
{
	if (!simple_empty(dentry))
		return -ENOTEMPTY;

	drop_nlink(d_inode(dentry));
	drop_nlink(dir);

	return BiscuitOS_unlink(dir, dentry);
}

static int BiscuitOS_whiteout(struct inode *old_dir, struct dentry *old_dentry)
{
	struct dentry *whiteout;
	int error;

	whiteout = d_alloc(old_dentry->d_parent, &old_dentry->d_name);
	if (!whiteout)
		return -ENOMEM;

	error = BiscuitOS_mknod(old_dir, whiteout,
			S_IFCHR | WHITEOUT_MODE, WHITEOUT_DEV);
	dput(whiteout);
	if (error)
		return error;

	/*
	 * Cheat and hash the whiteout while the old dentry is still in
	 * place, instead of playing games with FS_RENAME_DONE_D_MOVE.
	 *
	 * d_lookup() will consistently find one of them at this point,
	 * not sure which one, but that isn't even important.
	 */
	d_rehash(whiteout);
	return 0;
}

static int BiscuitOS_exchange(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry)
{
	bool old_is_dir = d_is_dir(old_dentry);
	bool new_is_dir = d_is_dir(new_dentry);

	if (old_dir != new_dir && old_is_dir != new_is_dir) {
		if (old_is_dir) {
			drop_nlink(old_dir);
			inc_nlink(new_dir);
		} else {
			drop_nlink(new_dir);
			inc_nlink(old_dir);
		}
	}
	old_dir->i_ctime = old_dir->i_mtime =
	new_dir->i_ctime = new_dir->i_mtime =
	d_inode(old_dentry)->i_ctime =
	d_inode(new_dentry)->i_ctime = current_time(old_dir);

	return 0;
}

/*
 * The VFS layer already does all the dentry stuff for rename,
 * we just have to decrement the usage count for the target if
 * it exists so that the VFS layer correctly fress's it when it
 * gets overwritten.
 */
static int BiscuitOS_rename(struct inode *old_dir, struct dentry *old_dentry,
	struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	struct inode *inode = d_inode(old_dentry);
	int they_are_dirs = S_ISDIR(inode->i_mode);

	if (flags & ~(RENAME_NOREPLACE | RENAME_EXCHANGE | RENAME_WHITEOUT))
		return -EINVAL;

	if (flags & RENAME_EXCHANGE)
		return BiscuitOS_exchange(old_dir, old_dentry, 
							new_dir, new_dentry);

	if (!simple_empty(new_dentry))
		return -ENOTEMPTY;

	if (flags & RENAME_WHITEOUT) {
		int error;

		error = BiscuitOS_whiteout(old_dir, old_dentry);
		if (error)
			return error;
	}

	if (d_really_is_positive(new_dentry)) {
		(void) BiscuitOS_unlink(new_dir, new_dentry);
		if (they_are_dirs) {
			drop_nlink(d_inode(new_dentry));
			drop_nlink(old_dir);
		}
	} else if (they_are_dirs) {
		drop_nlink(old_dir);
		inc_nlink(new_dir);
	}

	old_dir->i_size -= BOGO_DIRENT_SIZE;
	new_dir->i_size += BOGO_DIRENT_SIZE;
	old_dir->i_ctime = old_dir->i_mtime =
	new_dir->i_ctime = new_dir->i_mtime =
	inode->i_ctime = current_time(old_dir);
	return 0;
}

static int BiscuitOS_tmpfile(struct inode *dir, 
				struct dentry *dentry, umode_t mode)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static struct inode *BiscuitOS_get_inode(struct super_block *sb,
		const struct inode *dir, umode_t mode, dev_t dev,
		unsigned long flags)
{
	struct inode *inode;
	struct BiscuitOS_inode_info *info;
	struct BiscuitOS_sb_info *sbinfo = BISCUITOS_SB(sb);

	if (BiscuitOS_reserve_inode(sb))
		return NULL;

	inode = new_inode(sb);
	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_blocks = 0;
		inode->i_atime = current_time(inode);
		inode->i_mtime = inode->i_atime;
		inode->i_ctime = inode->i_atime;
		inode->i_generation = prandom_u32();
		info = BISCUITOS_I(inode);
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
			inode->i_op = &BiscuitOS_special_inode_operations;
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_mapping->a_ops = &BiscuitOS_aops;
			inode->i_op = &BiscuitOS_inode_operations;
			inode->i_fop = &BiscuitOS_file_operations;
			mpol_shared_policy_init(&info->policy,
					BiscuitOS_get_sbmpol(sbinfo));
			break;
		case S_IFDIR:
			inc_nlink(inode);
			/* Some things misbehave if size == 0 on a directory */
			inode->i_size = 2 * BOGO_DIRENT_SIZE;
			inode->i_op = &BiscuitOS_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;
			break;
		case S_IFLNK:
			/*
			 * Must not load anything in the rbtree,
			 * mpol_free_shared_policy will not be called.
			 */
			mpol_shared_policy_init(&info->policy, NULL);
			break;
		}

		lockdep_annotate_inode_mutex_key(inode);
	} else
		BiscuitOS_free_inode(sb);
	return inode;
}

static int BiscuitOS_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct BiscuitOS_sb_info *sbinfo;
	int err = -ENOMEM;

	/* Round up to L1_CACHE_BYTE to resist false sharing */
	sbinfo = kzalloc(max((int)sizeof(struct BiscuitOS_sb_info),
				L1_CACHE_BYTES), GFP_KERNEL);
	if (!sbinfo)
		return -ENOMEM;
	
	sbinfo->mode = 0777 | S_ISVTX;
	sbinfo->uid  = current_fsuid();
	sbinfo->gid  = current_fsgid();
	sb->s_fs_info = sbinfo;

	/*
	 * Per default we only allow half of the physical ram per
	 * BiscuitOS fs instance, limiting inode to one per pace of lowmem;
	 * but the internal instance is left unlimited.
	 */
	if (!(sb->s_flags & SB_KERNMOUNT)) {
		sbinfo->max_blocks = BiscuitOS_default_max_blocks();
		sbinfo->max_inodes = BiscuitOS_default_max_inodes();
	} else {
		/* Mount by kern_mount() */
		sb->s_flags |= SB_NOUSER;
	}
	sb->s_export_op = &BiscuitOS_export_ops;
	sb->s_flags |= SB_NOSEC;

	spin_lock_init(&sbinfo->stat_lock);
	if (percpu_counter_init(&sbinfo->used_blocks, 0, GFP_KERNEL))
		goto failed;
	sbinfo->free_inodes = sbinfo->max_inodes;
	spin_lock_init(&sbinfo->shrinklist_lock);
	INIT_LIST_HEAD(&sbinfo->shrinklist);

	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = BISCUITOS_FS_MAGIC;
	sb->s_op = &BiscuitOS_ops;
	sb->s_time_gran = 1;

	uuid_gen(&sb->s_uuid);

	inode = BiscuitOS_get_inode(sb, NULL, S_IFDIR | sbinfo->mode,
							0, VM_NORESERVE);
	if (!inode)
		goto failed;
	inode->i_uid = sbinfo->uid;
	inode->i_gid = sbinfo->gid;
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		goto failed;
	return 0;

failed:
	BiscuitOS_put_super(sb);
	return err;
}

static void BiscuitOS_put_link(void *arg)
{
	mark_page_accessed(arg);
	put_page(arg);
}

static const char *BiscuitOS_get_link(struct dentry *dentry,
			struct inode *inode, struct delayed_call *done)
{
	struct page *page = NULL;
	int error;
	if (!dentry) {
		page = find_get_page(inode->i_mapping, 0);
		if (!page)
			return ERR_PTR(-ECHILD);
		if (!PageUptodate(page)) {
			put_page(page);
			return ERR_PTR(-ECHILD);
		}
	} else {
		error = BiscuitOS_getpage(inode, 0, &page, BGP_READ);
		if (error)
			return ERR_PTR(error);
		unlock_page(page);
	}
	set_delayed_call(done, BiscuitOS_put_link, page);
	return page_address(page);
}

static struct dentry *BiscuitOS_mount(struct file_system_type *fs_type,
			int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, BiscuitOS_fill_super);
}

static const struct inode_operations BiscuitOS_symlink_inode_operations = {
	.get_link	= BiscuitOS_get_link,
};

static const struct inode_operations BiscuitOS_short_symlink_operations = {
	.get_link	= simple_get_link,
};

/* FS type */
static const struct inode_operations BiscuitOS_dir_inode_operations = {
	.create		= BiscuitOS_create,
	.lookup		= simple_lookup,
	.link		= BiscuitOS_link,
	.unlink		= BiscuitOS_unlink,
	.symlink	= BiscuitOS_symlink,
	.mkdir		= BiscuitOS_mkdir,
	.rmdir		= BiscuitOS_rmdir,
	.mknod		= BiscuitOS_mknod,
	.rename		= BiscuitOS_rename,
	.tmpfile	= BiscuitOS_tmpfile,
};

static const struct file_operations BiscuitOS_file_operations = {
	.mmap		= BiscuitOS_mmap,
	.llseek		= BiscuitOS_file_llseek,
	.read_iter	= BiscuitOS_file_read_iter,
	.write_iter	= generic_file_write_iter,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.fallocate	= BiscuitOS_fallocate,
	.get_unmapped_area = BiscuitOS_get_unmapped_area,
};

static const struct inode_operations BiscuitOS_inode_operations = {
	.getattr	= BiscuitOS_getattr,
	.setattr	= BiscuitOS_setattr,
};

static const struct address_space_operations BiscuitOS_aops = {
	.writepage	= BiscuitOS_writepage,
#ifndef CONFIG_COMPILE_MODULE
	.set_page_dirty	= __set_page_dirty_no_writeback,
#endif
	.write_begin	= BiscuitOS_write_begin,
	.write_end	= BiscuitOS_write_end,
	.migratepage	= migrate_page,
	.error_remove_page = generic_error_remove_page,
};

static const struct inode_operations BiscuitOS_special_inode_operations = {

};

static const struct export_operations BiscuitOS_export_ops = {
	.get_parent	= BiscuitOS_get_parent,
	.encode_fh	= BiscuitOS_encode_fh,
	.fh_to_dentry	= BiscuitOS_fh_to_dentry,
};

static const struct super_operations BiscuitOS_ops = {
	.alloc_inode	= BiscuitOS_alloc_inode,
	.destroy_inode	= BiscuitOS_destroy_inode,
	.statfs		= BiscuitOS_statfs,
	.remount_fs	= BiscuitOS_remount_fs,
	.show_options	= BiscuitOS_show_options,
	.evict_inode	= BiscuitOS_evict_inode,
	.drop_inode	= generic_delete_inode,
	.put_super	= BiscuitOS_put_super,
};

static struct file_system_type BiscuitOS_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "BiscuitOS_fs",
	.mount		= BiscuitOS_mount,
	.kill_sb	= kill_litter_super,
	.fs_flags	= FS_USERNS_MOUNT,
};

static __init int BiscuitOS_fs_init(void)
{
	int error;
	
	/* If rootfs called this, don't re-init */
	if (BiscuitOS_inode_cachep)
		return 0;

	/* alloc BiscuitOS inode cacahe */
	BiscuitOS_init_inodecache();

	/* Register filesystem */
	error = register_filesystem(&BiscuitOS_fs_type);
	if (error) {
		printk("Could not register BiscuitOS fs\n");
		goto out2;
	}

	/* Mounting BiscuitOS fs */
	BiscuitOS_mnt = kern_mount(&BiscuitOS_fs_type);
	if (IS_ERR(BiscuitOS_mnt)) {
		error = PTR_ERR(BiscuitOS_mnt);
		printk("Could not kern_mount BiscuitOS fs\n");
		goto out1;
	}

	return 0;

out1:
	unregister_filesystem(&BiscuitOS_fs_type);
out2:
	BiscuitOS_destroy_inodecache();
	BiscuitOS_mnt = ERR_PTR(error);
	return error;
}
#ifndef CONFIG_COMPILE_MODULE
fs_initcall(BiscuitOS_fs_init);
#else
#include <linux/module.h>
module_init(BiscuitOS_fs_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS Filesystem Module");
#endif
