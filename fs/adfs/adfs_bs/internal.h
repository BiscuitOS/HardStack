#ifndef _BISCUITOS_ADFS_H
#define _BISCUITOS_ADFS_H

/*
 * adfs file system inode data in memory
 */
struct adfs_inode_info {
	loff_t		mmu_private;
	unsigned long	parent_id;	/* object id of parent */
	__u32		loadaddr;	/* RISC OS load address */
	__u32		execaddr;	/* RISC OS exec address */
	unsigned int	filetype;	/* RISC OS file type */
	unsigned int	attr;		/* RISC OS permission */
	unsigned int	stamped:1;	/* RISC OS file has data/time */
	struct inode	vfs_inode;
};

#define BS_DUP()	printk("Expand..[%s-%d]\n", __func__, __LINE__)

#endif
