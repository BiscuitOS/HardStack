#ifndef _BISCUITOS_MINIXFS_H
#define _BISCUITOS_MINIXFS_H

#define INODE_VERSION(inode)	minix_sb_bs(inode->i_sb)->s_version
#define MINIX_V1_BS		0x0001		/* original minix fs */
#define MINIX_V2_BS		0x0002		/* minix V2 fs */
#define MINIX_V3_BS		0x0003		/* minix V3 fs */

/*
 * minix fs inode data in memory
 */
struct minix_inode_info {
	union {
		__u16 i1_data[16];
		__u32 i2_data[16];
	} u;
	struct inode vfs_inode;
};

/*
 * minix super-block data in memory
 */             
struct minix_sb_info {          
	unsigned long s_ninodes;
	unsigned long s_nzones;
	unsigned long s_imap_blocks;
	unsigned long s_zmap_blocks;
	unsigned long s_firstdatazone;
	unsigned long s_log_zone_size;
	unsigned long s_max_size;
	int s_dirsize;
	int s_namelen; 
	struct buffer_head ** s_imap;
	struct buffer_head ** s_zmap;
	struct buffer_head * s_sbh;
	struct minix_super_block * s_ms;
	unsigned short s_mount_state;
	unsigned short s_version;
};

#define minix_set_bit_bs(nr, addr)				\
	__set_bit((nr), (unsigned long *)(addr))

static inline unsigned minix_blocks_needed_bs(unsigned bits, 
							unsigned blocksize)
{
	return DIV_ROUND_UP(bits, blocksize * 8);
}

static inline struct minix_sb_info *minix_sb_bs(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct minix_inode_info *minix_i_bs(struct inode *inode)
{
	return container_of(inode, struct minix_inode_info, vfs_inode);
}

extern struct minix_inode * minix_V1_raw_inode_bs(struct super_block *sb, 
					ino_t ino, struct buffer_head **bh);
extern void mark_buffer_dirty(struct buffer_head *bh);
extern struct minix2_inode *minix_V2_raw_inode_bs(struct super_block *sb,
				ino_t ino, struct buffer_head **bh);
extern const struct file_operations minix_dir_operations_bs;

#define BS_DUP() printk("Expand..[%s][%s][%d]\n", __FILE__, __func__, __LINE__)

#endif
