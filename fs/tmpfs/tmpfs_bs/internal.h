#ifndef _BISCUITOS_TMPFS_H
#define _BISCUITOS_TMPFS_H

#include <linux/magic.h>

/* TMPFS_bs MAGIC */
#define TMPFS_MAGIC_BS		(TMPFS_MAGIC + 0x10000000)

/* Pretend that each entry is of this size in directory's i_size */
#define BOGO_DIRENT_SIZE	20

#define BLOCKS_PER_PAGE		(PAGE_SIZE/512)
#define VM_ACCT(size)		(PAGE_ALIGN(size) >> PAGE_SHIFT)

/*
 * Definitions for "huge tmpfs": tmpfs mounted with the huge= option
 *
 * SHMEM_HUGE_NEVER:
 *	disables huge pages for the mount;
 * SHMEM_HUGE_ALWAYS:
 *	enables huge pages for the mount;
 * SHMEM_HUGE_WITHIN_SIZE:
 *	only allocate huge pages if the page will be fully within i_size,
 *	also respect fadvise()/mdavise() hints;
 * SHMEM_HUGE_ADVISE:
 *	only allocate huge pages if requested with fadvise()/madvise();
 */
#define SHMEM_HUGE_NEVER	0
#define SHMEM_HUGE_ALWAYS	1
#define SHMEM_HUGE_WITHIN_SIZE	2
#define SHMEM_HUGE_ADVISE	3

/*
 * Special values.
 * Only can be set via /sys/kernel/mm/transparent_hugepage/shmem_enabled:
 *
 * SHMEM_HUGE_DENY:
 * 	disables huge on shm_mnt and all mounts, for emergency use;
 * SHMEM_HUGE_FORCE:
 *	enable huge on shm_mnt and all mounts, w/o needing option, for testing;
 */
#define SHMEM_HUGE_DENY		(-1)
#define SHMEM_HUGE_FORCE	(-2)

#define shmem_huge		SHMEM_HUGE_DENY
#define shmem_initxattrs_bs	NULL
#ifndef CONFIG_NUMA
#define vm_policy		vm_private_data
#endif

static inline struct shmem_sb_info *SHMEM_SB_BS(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct shmem_inode_info *SHMEM_I_BS(struct inode *inode)
{
	return container_of(inode, struct shmem_inode_info, vfs_inode);
}

extern struct inode *shmem_get_inode_bs(struct super_block *sb, 
	const struct inode *dir, umode_t mode, dev_t dev, unsigned long flags);

#define BS_DUP() printk("Expand. %s-%s-%d\n", __FILE__, __func__, __LINE__)

#endif
