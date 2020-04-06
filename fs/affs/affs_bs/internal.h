#ifndef _BISCUITOS_AFFS_H
#define _BISCUITOS_AFFS_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/fs.h>

struct affs_ext_key {
	u32	ext;	/* idx of the extended block */
	u32	key;	/* block number */
};

/*
 * affs fs inode data in memory
 */
struct affs_inode_info {
	atomic_t i_opencnt;
	struct semaphore i_link_lock;	/* Protects internal inode access. */
	struct semaphore i_ext_lock;	/* Protects internal inode access. */
#define i_hash_lock i_ext_lock
	u32	i_blkcnt;		/* block count */
	u32	i_extcnt;		/* extended block count */
	u32	*i_lc;			/* linear cache of extended blocks */
	u32	i_lc_size;
	u32	i_lc_shift;
	u32	i_lc_mask;
	struct affs_ext_key *i_ac; /* associative cache of extended blocks */
	u32	i_ext_last;	   /* last accessed extended block */
	struct buffer_head *i_ext_bh;	/* bh of last extended block */
	loff_t	mmu_private;
	u32	i_protect;		/* unused attribute bits */
	u32	i_lastalloc;		/* last allocated block */
	int	i_pa_cnt;		/* number of preallocated blocks */
	struct inode vfs_inode;
};

#define BS_DUP()	printk("Expand..[%s-%d]\n", __func__, __LINE__)

#endif
