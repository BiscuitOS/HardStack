#ifndef _BISCUITOS_TMPFS_H
#define _BISCUITOS_TMPFS_H

#include <linux/magic.h>

/* TMPFS_bs MAGIC */
#define TMPFS_MAGIC_BS		(TMPFS_MAGIC + 0x10000000)

/* Pretend that each entry is of this size in directory's i_size */
#define BOGO_DIRENT_SIZE	20

#define shmem_initxattrs_bs	NULL

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
