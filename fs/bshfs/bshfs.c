/*
 * BiscuitOS filesystem
 *
 * (C) 2019.12.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/iversion.h>
#include <linux/dax.h>
#include <linux/quotaops.h>
#include <linux/random.h>
#include <linux/namei.h>
#include <linux/exportfs.h>
#include <linux/buffer_head.h>
#include <linux/blockgroup_lock.h>

/* BiscuitOS-hfs Magic */
#define BISCUITOS_SUPER_MAGIC		0xEF53
#define BISCUITOS_LINK_MAX		32000
/* BiscuitOS-hfs version */
#define BISCUITOS_DATE			"19/12/12"
#define BISCUITOS_VERSION		"0.2b"

/*
 * Maximal mount counts between two filesystem checks
 */
#define BISCUITOS_DFL_MAX_MNT_COUNT	20	/* Allow 20 mounts */
#define BISCUITOS_DFL_CHECKINTERVAL	0	/* Don;t use interval check */

/*
 * Revision levels
 */
#define BISCUITOS_GOOD_OLD_REV		0 /* The good old (original) format*/
#define BISCUITOS_DYNAMIC_DEV		1 /* V2 format w/ dynamic inode sizes */
#define BISCUITOS_CURRENT_REV		BISCUITOS_GOOD_OLD_REV
#define BISCUITOS_MAX_SUPP_REV		BISCUITOS_DYNAMIC_REV

/*
 * Special inode numbers
 */
#define BISCUITOS_BAD_INO		1	/* Bad blocks inode */
#define BISCUITOS_ROOT_INO		2	/* Root inode */
#define BISCUITOS_ROOT_LOADER_INO	5	/* Boot loader inode */
#define BISCUITOS_UNDEL_DIR_INO		6	/* Undelete directory inode */

/*
 * Inode flags (GETFLAGS/SETFLAGS)
 */
#define BISCUITOS_SECRM_FL		FS_SECRM_FL	/* Secure deletion */
#define BISCUITOS_UNRM_FL		FS_UNRM_FL	/* Undelete */
#define BISCUITOS_COMPR_FL		FS_COMPR_FL	/* Compress file */
#define BISCUITOS_SYNC_FL		FS_SYNC_FL	/* Synchronous updtes */
#define BISCUITOS_IMMUTABLE_FL		FS_IMMUTABLE_FL	/* Immutable file */
#define BISCUITOS_APPEND_FL		FS_APPEND_FL	/* writs to file may only append */
#define BISCUITOS_NODUMP_FL		FS_NODUMP_FL	/* do not dump file */
#define BISCUITOS_NOATIME_FL		FS_NOATIME_FL	/* do not update atime*/
/* Reserved for compression usage... */
#define BISCUITOS_DIRTY_FL		FS_DIRTY_FL
#define BISCUITOS_COMPRBLK_FL		FS_COMPRBLK_FL	/* One or more compressed clusters */
#define BISCUITOS_NOCOMP_FL		FS_NOCOMP_FL	/* Don't compress */
#define BISCUITOS_ECOMPR_FL		FS_ECOMPR_FL	/* Compression error */
/* End compression flags --- maybe not all used */
#define BISCUITOS_BTREE_FL		FS_BTREE_FL	/* btree format dir */
#define BISCUITOS_INDEX_FL		FS_INDEX_FL	/* hash-indexed directory */
#define BISCUITOS_IMAGIC_FL		FS_IMAGIC_FL	/* AFS directory */
#define BISCUITOS_JOURNAL_DATA_FL	FS_JOURNAL_DATA_FL
#define BISCUITOS_NOTAIL_FL		FS_NOTAIL_FL	/* file tail should not be merged */
#define BISCUITOS_DIRSYNC_FL		FS_DIRSYNC_FL	/* dirsync behaviour (directories only) */
#define BISCUITOS_TOPDIR_FL		FS_TOPDIR_FL	/* Top of directory hierarchies */
#define BISCUITOS_RESERVED_FL		FS_RESERVED_FL	/* reserved for BiscuitOS lib */
#define BISCUITOS_FL_USER_VISIBLE	FS_FL_USER_VISIBLE	/* User visible flags */
#define BISCUITOS_FL_USER_MODIFIABLE	FS_FL_USER_MODIFIABLE	/* User modifiable flags */

/* Flags that should be inherited by new inodes from their parent. */
#define BISCUITOS_FL_INHERITED (BISCUITOS_SECRM_FL | BISCUITOS_UMRM_FL | \
				BISCUITOS_COMPR_FL | BISCUITOS_SYNC_FL | \
				BISCUITOS_NODUMP_FL | BISCUITOS_COMPRBLK_FL |\
				BISCUITOS_NOATIME_FL | BISCUITOS_NOCOMP_FL | \
				BISCUITOS_JOURNAL_DATA_FL | \
				BISCUITOS_NOTAIL_FL | BISCUITOS_DIRSYNC_FL)

/*
 * Default mount options
 */
#define BISCUITOS_DEFM_DEBUG		0x0001
#define BISCUITOS_DEFM_BSDGROUPS	0x0002
#define BISCUITOS_DEFM_XATTR_USER	0x0004
#define BISCUITOS_DEFM_ACL		0x0008
#define BISCUITOS_DEFM_UID16		0x0010

/*
 * Mount flags
 */
#define BISCUITOS_MOUNT_CHECK		0x000001 /* Do mount-time checks */
#define BISCUITOS_MOUNT_OKDALLOC	0x000002 /* Don't use the new Orlov allocator */
#define BISCUITOS_MOUNT_GRPID		0x000004 /* Create files with directory's group */
#define BISCUITOS_MOUNT_DEBUG		0x000008 /* Some debugging message */
#define BISCUITOS_MOUNT_ERRORS_CONT	0x000010 /* Continue on errors */
#define BISCUITOS_MOUNT_ERRORS_RO	0x000020 /* Remount fs ro on erros */
#define BISCUITOS_MOUNT_ERRORS_PANIC	0x000040 /* Panic on errors */
#define BISCUITOS_MOUNT_MINIX_DF	0x000080 /* Mimics the Minix statfs */
#define BISCUITOS_MOUNT_NOBH		0x000100 /* No buffer_heads */
#define BISCUITOS_MOUNT_NO_UID32	0x000200 /* Disable 32-bit UIDs */
#define BISCUITOS_MOUNT_XATTR_USER	0x004000 /* Extended user attribute */
#define BISCUITOS_MOUNT_POSIX_ACL	0x008000 /* POSIX Access Control Lists*/
#define BISCUITOS_MOUNT_XIP		0x010000 /* Obsolete, use DAX */
#define BISCUITOS_MOUNT_USRQUOTA	0x020000 /* user quota */
#define BISCUITOS_MOUNT_GRPQUOTA	0x040000 /* group quota */
#define BISCUITOS_MOUNT_RESERVATION	0x080000 /* Preallocation */
#define BISCUITOS_MOUNT_DAX		0x100000 /* Direct Access */

#define clear_opt(o, opt)		o &= ~BISCUITOS_MOUNT_##opt
#define set_opt(o, opt)			o |= BISCUITOS_MOUNT_##opt
#define test_opt(sb, opt)		(BISCUITOS_SB(sb)->s_mount_opt & \
					BISCUITOS_MOUNT_##opt)

/*
 * Behaviour when detecting errors
 */
#define BISCUITOS_ERRORS_CONTINUE	1	/* Continue execution */
#define BISCUITOS_ERRORS_RO		2	/* Remount fs read-only */
#define BISCUITOS_ERRORS_PANIC		3	/* Panic */
#define BISCUITOS_ERRORS_DEFAULT	BISCUITOS_ERRORS_CONTINUE

/*
 * Revision levels
 */
#define BISCUITOS_GOOD_OLD_REV		0 /* The good old (original) format */
#define BISCUITOS_DYNAMIC_REV		1 /* V2 format w/ dynamic inode sizes */

#define BISCUITOS_GOOD_OLD_INODE_SIZE	128

/* First non-reserved inode for old filesystem */
#define BISCUITOS_GOOD_OLD_FIRST_INO	11

/*
 * File system states
 */
#define BISCUITOS_VALID_FS		0x0001	/* Unmounted cleanly */
#define BISCUITOS_ERROR_FS		0x0002	/* Errors detected */
#define EFSCORRUPTED			EUCLEAN	/* Filesystem is corrupted */

/*
 * Feature set definitions
 */
#define BISCUITOS_HAS_COMPAT_FEATURE(sb,mask)				\
	(BISCUITOS_SB(sb)->s_bs->s_feature_compat & cpu_to_le32(mask))
#define BISCUITOS_HAS_RO_COMPAT_FEATURE(sb,mask)			\
	(BISCUITOS_SB(sb)->s_bs->s_feature_ro_compat & cpu_to_le32(mask))
#define BISCUITOS_HAS_INCOMPAT_FEATURE(sb,mask)				\
	(BISCUITOS_SB(sb)->s_bs->s_feature_incompat & cpu_to_le32(mask))
#define BISCUITOS_SET_COMPAT_FEATURE(sb,mask)				\
	BISCUITOS_SB(sb)->s_bs->s_feature_compat |= cpu_to_le32(mask)
#define BISCUITOS_SET_RO_COMPAT_FEATURE(sb,mask)			\
	BISCUITOS_SB(sb)->s_bs->s_feature_ro_compat |= cpu_to_le32(mask)
#define BISCUITOS_SET_INCOMPAT_FEATURE(sb,mask)				\
	BISCUITOS_SB(sb)->s_bs->s_feature_incompat |= cpu_to_le32(mask)
#define BISCUITOS_CLEAR_COMPAT_FEATURE(sb,mask)				\
	BISCUITOS_SB(sb)->s_bs->s_feature_compat &= ~cpu_to_le32(mask)
#define BISCUITOS_RO_COMPAT_FEATURE(sb,mask)				\
	BISCUITOS_SB(sb)->s_bs->s_feature_ro_compat &= ~cpu_to_le32(mask)
#define BISCUITOS_CLEAR_INCOMPAT_FEATURE(sb,mask)			\
	BISCUITOS_SB(sb)->s_bs->s_feature_incompat &= ~cpu_to_le32(mask)

#define BISCUITOS_FEATURE_COMPAT_DIR_PREALLOC		0x0001
#define BISCUITOS_FEATURE_COMPAT_IMAGIC_INODES		0x0002
#define BISCUITOS_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define BISCUITOS_FEATURE_COMPAT_EXT_ATTR		0x0008
#define BISCUITOS_FEATURE_COMPAT_RESIZE_INO		0x0010
#define BISCUITOS_FEATURE_COMPAT_DIR_INDEX		0x0020
#define BISCUITOS_FEATURE_COMPAT_ANY			0xffffffff

#define BISCUITOS_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define BISCUITOS_FEATURE_RO_COMPAT_LARGE_FILE		0x0002
#define BISCUITOS_FEATURE_RO_COMPAT_BTREE_DIR		0x0004
#define BISCUITOS_FEATURE_RO_COMPAT_ANY

#define BISCUITOS_FEATURE_INCOMPAT_COMPRESSION		0x0001
#define BISCUITOS_FEATURE_INCOMPAT_FILETYPE		0x0002
#define BISCUITOS_FEATURE_INCOMPAT_RECOVER		0x0004
#define BISCUITOS_FEATURE_INCOMPAT_JOURNAL_DEV		0x0008
#define BISCUITOS_FEATURE_INCOMPAT_META_BG		0x0010
#define BISCUITOS_FEATURE_INCOMPAT_NAY			0xffffffff

#define BISCUITOS_FEATURE_COMPAT_SUPP	BISCUITOS_FEATURE_COMPAT_EXT_ATTR
#define BISCUITOS_FEATURE_INCOMPAT_SUPP					\
				(BISCUITOS_FEATURE_INCOMPAT_FILETYPE| \
				 BISCUITOS_FEATURE_INCOMPAT_META_BG)
#define BISCUITOS_FEATURE_RO_COMPAT_SUPP 				\
				(BISCUITOS_FEATURE_RO_COMPAT_SPARSE_SUPER | \
				 BISCUITOS_FEATURE_RO_COMPAT_LARGE_FILE | \
				 BISCUITOS_FEATURE_RO_COMPAT_BTREE_DIR)
#define BISCUITOS_FEATURE_RO_COMPAT_UNSUPPORTED				\
				~BISCUITOS_FEATURE_RO_COMPAT_SUPP
#define BISCUITOS_FEATURE_INCOMPAT_UNSUPPORTED				\
				~BISCUITOS_FEATURE_INCOMPAT_SUPP

/*
 * Macro-instructions used to manage several block sizes
 */
#define BISCUITOS_MIN_BLOCK_SIZE	1024
#define BISCUITOS_MAX_BLOCK_SIZE	4096
#define BISCUITOS_MIN_BLOCK_LOG_SIZE	10
#define BISCUITOS_BLOCK_SIZE(s)		((s)->s_blocksize)
#define BISCUITOS_ADDR_PER_BLOCK(s)	(BISCUITOS_BLOCK_SIZE(s)/sizeof(__u32))
#define BISCUITOS_BLOCK_SIZE_BITS(s)	((s)->s_blocksize_bits)
#define BISCUITOS_ADDR_PER_BLOCK_BITS(s)				\
					(BISCUITOS_SB(s)->s_addr_per_block_bits)
#define BISCUITOS_INODE_SIZE(s)		(BISCUITOS_SB(s)->s_inode_size)
#define BISCUITOS_FIRST_INO(s)		(BISCUITOS_SB(s)->s_first_ino)

/*
 * Macro-instructions used to manage fragments
 */
#define BISCUITOS_MIN_FRAG_SIZE		1024
#define BISCUITOS_MAX_FRAG_SIZE		4096
#define BISCUITOS_MIN_FRAG_LOG_SIZE	10
#define BISCUITOS_FRAG_SIZE(s)		(BISCUITOS_SB(s)->s_frag_size)
#define BISCUITOS_FRAGS_PER_BLOCK(s)	(BISCUITOS_SB(s)->s_frags_per_block)

/*
 * Structure of a blocks group descriptor
 */
struct BiscuitOS_group_desc
{
	__le32	bg_block_bitmap;		/* Blocks bitmap block */
	__le32	bg_inode_bitmap;		/* Inodes bitmap block */
	__le32	bg_inode_table;			/* Inodes table block */
	__le16	bg_free_blocks_count;		/* Free blocks count */
	__le16	bg_free_inodes_count;		/* Free inodes count */
	__le16	bg_used_dirs_count;		/* Directories count */
	__le16	bg_pad;
	__le32	bg_reserved[3];
};

/*
 * Macro-instructions used to manage group descriptors
 */
#define BISCUITOS_BLOCKS_PER_GROUP(s)	(BISCUITOS_SB(s)->s_blocks_per_group)
#define BISCUITOS_DESC_PER_BLOCK(s)	(BISCUITOS_SB(s)->s_desc_per_block)
#define BISCUITOS_INODES_PER_GROUP(s)	(BISCUITOS_SB(s)->s_inodes_per_group)
#define BISCUITOS_DESC_PER_BLOCK_BITS(s)				\
					(BISCUITOS_SB(s)->s_desc_per_block_bits)

/*
 * Define BISCUITOS_RESERVATION to reserve data blocks for expanding files
 */
#define BISCUITOS_DEFAULT_RESERVE_BLOCKS	8
/* max window size: 1024(direct blocks) + 3([t,d]indirect blocks) */
#define BISCUITOS_MAX_RESERVE_BLOCKS		1027
#define BISCUITOS_RESERVE_WINDOW_NOT_ALLOCATED	0

/* data type for filesystem-wide blocks number */
typedef unsigned long BiscuitOS_fsblk_t;

struct BiscuitOS_reserve_window {
	BiscuitOS_fsblk_t	_rsv_start;	/* First byte reserved */
	BiscuitOS_fsblk_t	_rsv_end;	/* Last byte reserved or 0 */
};

struct BiscuitOS_reserve_window_node {
	struct rb_node	rsv_node;
	__u32		rsv_goal_size;
	__u32		rsv_alloc_hit;
	struct BiscuitOS_reserve_window		rsv_window;
};

struct BiscuitOS_block_alloc_info {
	/* information about reservation window */
	struct BiscuitOS_reserve_window_node rsv_window_node;
	/*
	 * was i_next_alloc_block in BiscuitOS_inode_info
	 * is the logical (file-relative) number of the
	 * most-recently-allocated block in this file.
	 * We use this for detecting linearly ascending allocation requests.
	 */
	__u32		last_alloc_logical_block;
	/*
	 * Was i_next_alloc_goal in BiscuitOS_inode_info
	 * is the *physical* companion to i_next_alloc_block,
	 * it the the physical block number of the block which was
	 * most-recentl allocated to this file. This give us the
	 * goal (target) for the next allocation when we detect
	 * linearly ascending requests.
	 */
	BiscuitOS_fsblk_t	last_alloc_physical_block;
};

#define rsv_start	rsv_window._rsv_start
#define rsv_end		rsv_window._rsv_end

/*
 * BiscuitOS mount options
 */
struct BiscuitOS_mount_options {
	unsigned long s_mount_opt;
	kuid_t s_resuid;
	kgid_t s_resgid;
};

/*
 * Constants relative to the data blocks
 */
#define BISCUITOS_NDIR_BLOCKS		12
#define BISCUITOS_IND_BLOCK		BISCUITOS_NDIR_BLOCKS
#define BISCUITOS_DIND_BLOCK		(BISCUITOS_IND_BLOCK + 1)
#define BISCUITOS_TIND_BLOCK		(BISCUITOS_DIND_BLOCK + 1)
#define BISCUITOS_N_BLOCKS		(BISCUITOS_TIND_BLOCK + 1)

/*
 * BiscuitOS hfs inode data in memory
 */
struct BiscuitOS_inode_info {
	__le32		i_data[15];
	__u32		i_flags;
	__u32		i_faddr;
	__u8		i_frag_no;
	__u8		i_frag_size;
	__u16		i_state;
	__u32		i_file_acl;
	__u32		i_dir_acl;
	__u32		i_dtime;

	/*
	 * i_block_group is the number of the block group which contains
	 * this file's inode. Constant across the lifetime of the inode,
	 * it is used for making block allocation decisions - we try to
	 * place a file's data blocks near its inode block, and new inodes
	 * near to their parent directory's inode.
	 */
	__u32		i_block_group;

	/* block reservation info */
	struct BiscuitOS_block_alloc_info *i_block_alloc_info;

	__u32		i_dir_start_lookup;
	rwlock_t	i_meta_lock;

	/*
	 * truncate_mutex is for serialising BiscuitOS_truncate() against
	 * BiscuitOS_getblock(). It also protects the internals of the inode's
	 * reservation data structures: BiscuitOS_reserve_window and
	 * BiscuitOS_reserve_window_node.
	 */
	struct mutex	truncate_mutex;
	struct inode	vfs_inode;
	struct list_head i_orphan;	/* unlinked but open inodes */
};

/*
 * BiscuitOS-hfs super-block data in memory
 */
struct BiscuitOS_sb_info {
	unsigned long s_frag_size;	/* Size of a fragment in bytes */
	unsigned long s_frags_per_block;/* Number of fragments per block */
	unsigned long s_inodes_per_block;/* Number of inodes per block */
	unsigned long s_frags_per_group;/* Number of fragments in a group */
	unsigned long s_blocks_per_group;/* Number of blocks in a group */
	unsigned long s_inodes_per_group;/* Number of inodes in a group */
	unsigned long s_itb_per_group;	/*Number of inode tables blocks per group */
	unsigned long s_gdb_count;	/* Number of group descriptor blocks */
	unsigned long s_desc_per_block; /* Number of group descriptors per block*/
	unsigned long s_groups_count;	/* Number of groups in the fs */
	unsigned long s_overhead_last;	/* Last calculated overhead */
	unsigned long s_blocks_last;	/* Last seen block count */
	struct buffer_head *s_sbh;	/* Buffer containing the super block */
	struct BiscuitOS_super_block *s_bs; /* Pointer to the super block in the buffer */
	struct buffer_head **s_group_desc;
	unsigned long s_mount_opt;
	unsigned long s_sb_block;
	kuid_t s_resuid;
	kgid_t s_resgid;
	unsigned short s_mount_state;
	unsigned short s_pad;
	int s_addr_per_block_bits;
	int s_desc_per_block_bits;
	int s_inode_size;
	int s_first_ino;
	spinlock_t s_next_gen_lock;
	u32 s_next_generation;
	unsigned long s_dir_count;
	u8 *s_debts;
	struct percpu_counter s_freeblocks_counter;
	struct percpu_counter s_freeinodes_counter;
	struct percpu_counter s_dirs_counter;
	struct blockgroup_lock *s_blockgroup_lock;
	/* root of the per fs reservation window tree */
	spinlock_t s_rsv_window_lock;
	struct rb_root s_rsv_window_root;
	struct BiscuitOS_reserve_window_node s_rsv_window_head;
	spinlock_t s_lock;
	struct mb_cache *s_ea_block_cache;
	struct dax_device *s_daxdev;
};

/*
 * BiscuitOS-hfs super block
 */
struct BiscuitOS_super_block {
	__le32	s_inodes_count;		/* Inodes count */
	__le32	s_blocks_count;		/* Blocks count */
	__le32	s_r_blocks_count;	/* Reserved blocks count */
	__le32	s_free_blocks_count;	/* Free blocks count */
	__le32	s_free_inodes_count;	/* Free inodes count */
	__le32	s_first_data_block;	/* First Data Block */
	__le32	s_log_block_size;	/* Block size */
	__le32	s_log_frag_size;	/* Fragment size */
	__le32	s_blocks_per_group;	/* # Blocks per group */
	__le32	s_frags_per_group;	/* # Fragments per group */
	__le32	s_inodes_per_group;	/* # Inode per group */
	__le32	s_mtime;		/* Mount time */
	__le32	s_wtime;		/* Write time */
	__le16	s_mnt_count;		/* Mount count */
	__le16	s_max_mnt_count;	/* Maximal mount count */
	__le16	s_magic;		/* Magic signature */
	__le16	s_state;		/* File system state */
	__le16	s_errors;		/* Behaviour when detecting errors */
	__le16	s_minor_rev_level;	/* minor revision level */
	__le32	s_lastcheck;		/* time of last check */
	__le32	s_checkinterval;	/* max. time between checks */
	__le32	s_creator_os;		/* OS */
	__le32	s_rev_level;		/* Revision level */
	__le16	s_def_resuid;		/* Default uid for reserved blocks */
	__le16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for BISCUITOS_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	__le32	s_first_ino;		/* First non-reserved inode */
	__le16	s_inode_size;		/* size of inode structure */
	__le16	s_block_group_nr;	/* block group # of this superblock */
	__le32	s_feature_compat;	/* compatible feature set */
	__le32	s_feature_incompat;	/* incompatible feature set */
	__le32	s_feature_ro_compat;	/*  readonly-compatible feature set */
	__u8	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16];	/* volume name */
	char	s_last_mounted[64];	/* directory where last mounted */
	__le32	s_alogrithm_usage_bitmap;	/* For compression */
	/*
	 * Performance hints. Directory preallocation should only
	 * happen if the BISCUITOS_COMPAT_PREALLOC flag is on.
	 */
	__u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	__u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	__u16	s_padding1;
	/*
	 * Journling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set
	 */
	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
	__u32	s_journal_inum;		/* inode number of journal file */
	__u32	s_journal_dev;		/* device number of journal file */
	__u32	s_last_orphan;		/* start of list of inodes to delete */
	__u32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_reserved_char_pad;
	__u16	s_reserved_word_pad;
	__le32	s_default_mount_opts;
	__le32	s_first_meta_bg;
	__u32	s_reserved[190];
};

/*
 * Structure of an inode on the disk
 */
struct BiscuitOS_inode {
	__le16	i_mode;		/* File mode */
	__le16	i_uid;		/* Low 16 bits of Owner Uid */
	__le32	i_size;		/* Size in bytes */
	__le32	i_atime;	/* Access time */
	__le32	i_ctime;	/* Creation time */
	__le32	i_mtime;	/* Modification time */
	__le32	i_dtime;	/* Deletion Time */
	__le16	i_gid;		/* Low 16 bits of Group Id */
	__le16	i_links_count;	/* Links count */
	__le32	i_blocks;	/* Blocks count */
	__le32	i_flags;	/* File flags */
	union {
		struct {
			__le32 l_i_reserved1;
		} linux1;
		struct {
			__le32 h_i_translator;
		} hurd1;
		struct {
			__le32 m_i_reserved1;
		} masix1;
	} osd1;			/* OS dependent 1 */
	__le32	i_block[BISCUITOS_N_BLOCKS];	/* Pointers to blocks */
	__le32	i_generation;	/* File version (for NFS) */
	__le32	i_file_acl;	/* File ACL */
	__le32	i_dir_acl;	/* Directory ACL */
	__le32	i_faddr;	/* Fragment address */
	union {
		struct {
			__u8	l_i_frag;	/* Fragment number */
			__u8	l_i_fsize;	/* Fragment size */
			__u16	i_pad1;
			__le16	l_i_uid_high;	/* these 2 fields */
			__le16	l_i_gid_high;	/* were reserved2[0] */
			__u32	l_i_reserved2;
		} linux2;
		struct {
			__u8	h_i_frag;	/* Fragment number */
			__u8	h_i_fsize;	/* Fragment size */
			__le16	h_i_mode_high;
			__le16	h_i_uid_high;
			__le16	h_i_gid_high;
			__le32	h_i_author;
		} hurd2;
		struct {
			__u8	m_i_frag;	/* Fragment number */
			__u8	m_i_fsize;	/* Fragment size */
			__u16	m_pad1;
			__u32	m_i_reserved2[2];
		} masix2;
	} osd2;			/* OS dependent 2 */
};

#define i_size_high	i_dir_acl
#define i_reserved1	osd1.linux1.l_i_reserved1
#define i_frag		osd2.linux2.l_i_frag
#define i_fsize		osd2.linux2.l_i_fsize
#define i_uid_low	i_uid
#define i_gid_low	i_gid
#define i_uid_high	osd2.linux2.l_i_uid_high
#define i_gid_high	osd2.linux2.l_i_gid_high
#define i_reserved2	osd2.linux2.l_i_reserved2

static const struct super_operations BiscuitOS_sops;
static const struct export_operations BiscuitOS_export_ops;
static const struct inode_operations BiscuitOS_file_inode_operations;
static const struct file_operations BiscuitOS_file_operations;
static const struct inode_operations BiscuitOS_dir_inode_operations;
static const struct inode_operations BiscuitOS_fast_symlink_inode_operations;
static const struct file_operations BiscuitOS_dir_operations;
static const struct inode_operations BiscuitOS_special_inode_operations;
static const struct inode_operations BiscuitOS_symlink_inode_operations;
static const struct address_space_operations BiscuitOS_nobh_aops;
static const struct address_space_operations BiscuitOS_aops;
static const struct address_space_operations BiscuitOS_dax_aops;
#define BiscuitOS_xattr_handlers	NULL

static struct kmem_cache *BiscuitOS_inode_cachep;

static inline struct BiscuitOS_sb_info *BISCUITOS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct BiscuitOS_inode_info *BISCUITOS_I(struct inode *inode)
{
	return container_of(inode, struct BiscuitOS_inode_info, vfs_inode);
}

static inline BiscuitOS_fsblk_t
BiscuitOS_group_first_block_no(struct super_block *sb, unsigned long group_no)
{
	return group_no * (BiscuitOS_fsblk_t)BISCUITOS_BLOCKS_PER_GROUP(sb) +
		le32_to_cpu(BISCUITOS_SB(sb)->s_bs->s_first_data_block);
}

static void BiscuitOS_init_once(void *foo)
{
	struct BiscuitOS_inode_info *info = (struct BiscuitOS_inode_info *)foo;

	rwlock_init(&info->i_meta_lock);
	mutex_init(&info->truncate_mutex);
	inode_init_once(&info->vfs_inode);
}

static int __init BiscuitOS_init_inodecache(void)
{
	BiscuitOS_inode_cachep = kmem_cache_create_usercopy(
			"BiscuitOS_inode_cacahe",
			sizeof(struct BiscuitOS_inode_info),
			0,
			(SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD|SLAB_ACCOUNT),
			offsetof(struct BiscuitOS_inode_info, i_data),
			sizeof_field(struct BiscuitOS_inode_info, i_data),
			BiscuitOS_init_once);
	if (BiscuitOS_inode_cachep == NULL)
		return -ENOMEM;
	return 0;
}

static void BiscuitOS_destroy_inodecache(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(BiscuitOS_inode_cachep);
}

static unsigned long get_sb_block(void **data)
{
	unsigned long sb_block;
	char *options = (char *)*data;

	if (!options || strncmp(options, "sb=", 3) != 0)
		return 1;
	options += 3;
	sb_block = simple_strtoul(options, &options, 0);
	if (*options && *options != ',') {
		printk("BiscuitOS-hfs: Invalid sb specification: %s\n",
						(char *)*data);
		return 1;
	}
	if (*options == ',')
		options++;
	*data = (void *)options;
	return sb_block;
}

static int parse_options(char *options, struct super_block *sb,
				struct BiscuitOS_mount_options *opts)
{
	if (!options)
		return 1;

	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

/*
 * Maximal file size. There is a direct, and {,double-,triple-}indirect
 * block limit, and also a limit of (2^32 - 1) 512-byte sectors in i_blocks.
 * We need to be 1 filesystem block less than the 2^32 sector limit.
 */
static loff_t BiscuitOS_max_size(int bits)
{
	loff_t res = BISCUITOS_NDIR_BLOCKS;
	int meta_blocks;
	loff_t upper_limit;

	/* This is calculated to be the largest files size for a
	 * dense, file such that the total number of
	 * sectors in the file, including data and all indirect blocks,
	 * does not exceed 2^32 - 1
	 * __u32 i_blocks representing the total number of
	 * 512 bytes blocks of the file
	 */
	upper_limit = (1LL << 32) - 1;

	/* total blocks in file system block size */
	upper_limit >>= (bits - 9);

	/* indirect blocks */
	meta_blocks = 1;
	/* double indirect blocks */
	meta_blocks += 1 + (1LL << (bits - 2));
	/* tripple indirect blocks */
	meta_blocks += 1 + (1LL << (bits - 2)) + (1LL << (2*(bits-2)));

	upper_limit -= meta_blocks;
	upper_limit <<= bits;

	res += 1LL << (bits - 2);
	res += 1LL << (2 * (bits - 2));
	res += 1LL << (3 * (bits - 2));
	res <<= bits;
	if (res > upper_limit)
		res = upper_limit;

	if (res > MAX_LFS_FILESIZE)
		res = MAX_LFS_FILESIZE;

	return res;
}

static inline int test_root(int a, int b)
{
	int num = b;

	while (a > num)
		num *= b;
	return num == a;
}

static int BiscuitOS_group_sparse(int group)
{
	if (group <= 1)
		return 1;
	return (test_root(group, 3) || test_root(group, 5) ||
			test_root(group, 7));
}

static int BiscuitOS_bg_has_super(struct super_block *sb, int group)
{
	if (BISCUITOS_HAS_RO_COMPAT_FEATURE(sb, 
			BISCUITOS_FEATURE_RO_COMPAT_SPARSE_SUPER) && 
					!BiscuitOS_group_sparse(group))
		return 0;
	return 1;
}

static unsigned long descriptor_loc(struct super_block *sb,
				unsigned long logic_sb_block, int nr)
{
	struct BiscuitOS_sb_info *sbi = BISCUITOS_SB(sb);
	unsigned long bg, first_meta_bg;
	int has_super = 0;

	first_meta_bg = le32_to_cpu(sbi->s_bs->s_first_meta_bg);

	if (!BISCUITOS_HAS_INCOMPAT_FEATURE(sb, 
		BISCUITOS_FEATURE_INCOMPAT_META_BG) || nr < first_meta_bg)
		return (logic_sb_block + nr + 1);
	bg = sbi->s_desc_per_block * nr;
	if (BiscuitOS_bg_has_super(sb, bg))
		has_super = 1;

	return BiscuitOS_group_first_block_no(sb, bg) + has_super;
}

static struct BiscuitOS_group_desc *BiscuitOS_get_group_desc(
		struct super_block *sb, unsigned int block_group, 
					struct buffer_head **bh)
{
	unsigned long group_desc;
	unsigned long offset;
	struct BiscuitOS_group_desc *desc;
	struct BiscuitOS_sb_info *sbi = BISCUITOS_SB(sb);

	if (block_group >= sbi->s_groups_count) {
		printk("%s block_group >= groups_count - "
			"block_group = %d, groups_count = %lu\n", __func__,
			block_group, sbi->s_groups_count);
		return NULL;
	}

	group_desc = block_group >> BISCUITOS_DESC_PER_BLOCK_BITS(sb);
	offset = block_group & (BISCUITOS_DESC_PER_BLOCK(sb) - 1);
	if (!sbi->s_group_desc[group_desc]) {
		printk("%s Group descriptor not loaded - blocks_group = "
			"%d, group_desc = %lu, desc = %lu", __func__,
			block_group, group_desc, offset);
		return NULL;
	}

	desc = 
	(struct BiscuitOS_group_desc *)sbi->s_group_desc[group_desc]->b_data;
	if (bh)
		*bh = sbi->s_group_desc[group_desc];
	return desc + offset;
}

static int BiscuitOS_check_descriptors(struct super_block *sb)
{
	int i;
	struct BiscuitOS_sb_info *sbi = BISCUITOS_SB(sb);

	//printk(KERN_DEBUG "Checking group descriptors\n");

	for (i = 0; i < sbi->s_groups_count; i++) {
		struct BiscuitOS_group_desc *gdp = 
					BiscuitOS_get_group_desc(sb, i, NULL);
		BiscuitOS_fsblk_t first_block = 
					BiscuitOS_group_first_block_no(sb, i);
		BiscuitOS_fsblk_t last_block;

		if (i == sbi->s_groups_count - 1)
			last_block = le32_to_cpu(sbi->s_bs->s_blocks_count) - 1;
		else
			last_block = first_block +
					(BISCUITOS_BLOCKS_PER_GROUP(sb) - 1);

		if (le32_to_cpu(gdp->bg_block_bitmap) < first_block ||
		    le32_to_cpu(gdp->bg_block_bitmap) > last_block) {
			printk("%s Block bitmap for group %d"
				" not in group (block %lu)\n", __func__, i, 
			(unsigned long)le32_to_cpu(gdp->bg_block_bitmap));
			return 0;
		}
		if (le32_to_cpu(gdp->bg_inode_bitmap) < first_block ||
		    le32_to_cpu(gdp->bg_inode_bitmap) > last_block) {
			printk("%s Inode bitmap for group %d not in"
				" group (block %lu)!\n", __func__, i,
			(unsigned long)le32_to_cpu(gdp->bg_inode_bitmap));
		}
		if (le32_to_cpu(gdp->bg_inode_table) < first_block ||
		    le32_to_cpu(gdp->bg_inode_table) + 
				sbi->s_itb_per_group - 1 > last_block) {
			printk("%s Inode table for group %d not in "
				"group (block %lu)!\n", __func__, i,
			(unsigned long)le32_to_cpu(gdp->bg_inode_table));
			return 0;
		}
	}
	return 1;
}

/**
 * __rsv_window_dump() -- Dump the filesystem block allocation reservation map
 * @rb_root:            root of per-filesystem reservation rb tree
 * @verbose:            verbose mode
 * @fn:                 function which wishes to dump the reservation map
 *
 * If verbose is turned on, it will print the whole block reservation
 * windows(start, end). Otherwise, it will only print out the "bad" windows,
 * those windows that overlap with their immediate neighbors.
 */
static void __rsv_window_dump(struct rb_root *root, int verbose,
				const char *fn)
{
	struct rb_node *n;
	struct BiscuitOS_reserve_window_node *rsv, *prev;
	int bad;

restart:
	n = rb_first(root);
	bad = 0;
	prev = NULL;

	printk("Block Allocation Reservation Windows Map (%s):\n", fn);
	while (n) {
		rsv = rb_entry(n, struct BiscuitOS_reserve_window_node, 
								rsv_node);
		if (verbose)
			printk("reservation window 0x%p start: %lu, "
			"end: %lu\n", rsv, rsv->rsv_start, rsv->rsv_end);
		if (rsv->rsv_start && rsv->rsv_start >= rsv->rsv_end) {
			printk("Bad reservation %p (start >= end)\n", rsv);
			bad = 1;
		}
		if (bad) {
			if (!verbose) {
				printk("Restarting reservation walk in "
					"verbose mode\n");
				verbose = 1;
				goto restart;
			}
		}
		n = rb_next(n);
		prev = rsv;
	}
	printk("Window map complete.\n");
	BUG_ON(bad);
}

#define rsv_window_dump(root, verbose)					\
	__rsv_window_dump((root), (verbose), __func__)

/*
 * BiscuitOS_rsv_window_add() -- Insert a window to the block reservation 
 *                               rb tree.
 * @sb:                 super block
 * @rsv:                reservation window to add
 *
 * Must be called with rsv_lock held.
 */
static void BiscuitOS_rsv_window_add(struct super_block *sb,
		struct BiscuitOS_reserve_window_node *rsv)
{
	struct rb_root *root = &BISCUITOS_SB(sb)->s_rsv_window_root;
	struct rb_node *node = &rsv->rsv_node;
	BiscuitOS_fsblk_t start = rsv->rsv_start;
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct BiscuitOS_reserve_window_node *this;

	while (*p) {
		parent = *p;
		this = rb_entry(parent, struct BiscuitOS_reserve_window_node,
					rsv_node);
		if (start < this->rsv_start)
			p = &(*p)->rb_left;
		else if (start > this->rsv_end)
			p = &(*p)->rb_right;
		else {
			rsv_window_dump(root, 1);
			BUG();
		}
	}
	rb_link_node(node, parent, p);
	rb_insert_color(node, root);
}

static unsigned long BiscuitOS_count_free_blocks(struct super_block *sb)
{
	struct BiscuitOS_group_desc *desc;
	unsigned long desc_count = 0;
	int i;

	for (i = 0; i < BISCUITOS_SB(sb)->s_groups_count; i++) {
		desc = BiscuitOS_get_group_desc(sb, i, NULL);
		if (!desc)
			continue;
		desc_count += le16_to_cpu(desc->bg_free_blocks_count);
	}
	return desc_count;
}

static unsigned long BiscuitOS_count_free_inodes(struct super_block *sb)
{
	struct BiscuitOS_group_desc *desc;
	unsigned long desc_count = 0;
	int i;

	for (i = 0; i < BISCUITOS_SB(sb)->s_groups_count; i++) {
		desc = BiscuitOS_get_group_desc(sb, i, NULL);
		if (!desc)
			continue;
		desc_count += le16_to_cpu(desc->bg_free_inodes_count);
	}
	return desc_count;
}

/* Called at mount-time, super-block is locked */
static unsigned long BiscuitOS_count_dirs(struct super_block *sb)
{
	unsigned long count = 0;
	int i;

	for (i = 0; i < BISCUITOS_SB(sb)->s_groups_count; i++) {
		struct BiscuitOS_group_desc *gdp =
				BiscuitOS_get_group_desc(sb, i, NULL);
		if (!gdp)
			continue;
		count += le16_to_cpu(gdp->bg_used_dirs_count);
	}
	return count;
}

static inline void BiscuitOS_xattr_destroy_cache(struct mb_cache *cache)
{
}

/** Super operations **/

static struct inode *BiscuitOS_alloc_inode(struct super_block *sb)
{
	struct BiscuitOS_inode_info *bi;

	bi = kmem_cache_alloc(BiscuitOS_inode_cachep, GFP_KERNEL);
	if (!bi)
		return NULL;
	bi->i_block_alloc_info = NULL;
	inode_set_iversion(&bi->vfs_inode, 1);

	return &bi->vfs_inode;
}

static void BiscuitOS_destroy_inode(struct inode *inode)
{
	printk("\n\n\n%s\n\n", __func__);
}

static int BiscuitOS_write_inode(struct inode *inode, 
				struct writeback_control *wbc)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static void BiscuitOS_evict_inode(struct inode *inode)
{
	printk("\n\n\n%s\n\n", __func__);
}

static void BiscuitOS_put_super(struct super_block *sb)
{
	printk("\n\n\n%s\n\n", __func__);
}

static void BiscuitOS_clear_super_error(struct super_block *sb)
{
	struct buffer_head *sbh = BISCUITOS_SB(sb)->s_sbh;

	if (buffer_write_io_error(sbh)) {
		/*
		 * Oh, dear. A previous attempt to write the 
		 * superblock failed. This could happen because the
		 * USB device wasw yanked out. Or it could happen to
		 * be a transient write error and maybe the block will
		 * be remapped. Nothing we can do but the retry the
		 * write and hope for the best.
		 */
		printk("previous I/O error to superblock detected.\n");
		clear_buffer_write_io_error(sbh);
		set_buffer_uptodate(sbh);
	}
}

static void BiscuitOS_sync_super(struct super_block *sb, 
			struct BiscuitOS_super_block *bs, int wait)
{
	BiscuitOS_clear_super_error(sb);
	spin_lock(&BISCUITOS_SB(sb)->s_lock);
	bs->s_free_blocks_count = cpu_to_le32(BiscuitOS_count_free_blocks(sb));
	bs->s_free_inodes_count = cpu_to_le32(BiscuitOS_count_free_inodes(sb));
	bs->s_wtime = cpu_to_le32(ktime_get_real_seconds());
	/* unlock before we do IO */
	spin_unlock(&BISCUITOS_SB(sb)->s_lock);
	mark_buffer_dirty(BISCUITOS_SB(sb)->s_sbh);
	if (wait)
		sync_dirty_buffer(BISCUITOS_SB(sb)->s_sbh);
}

static int BiscuitOS_sync_fs(struct super_block *sb, int wait)
{
	struct BiscuitOS_sb_info *sbi = BISCUITOS_SB(sb);
	struct BiscuitOS_super_block *bs = BISCUITOS_SB(sb)->s_bs;

	/*
	 * Write quota structures to quoto file, sync_blockdev() will write
	 * them to disk later.
	 */
	dquot_writeback_dquots(sb, -1);

	spin_lock(&sbi->s_lock);
	if (bs->s_state & cpu_to_le16(BISCUITOS_VALID_FS)) {
		printk("setting valid to 0\n");
		bs->s_state &= cpu_to_le16(~BISCUITOS_VALID_FS);
	}
	spin_unlock(&sbi->s_lock);
	BiscuitOS_sync_super(sb, bs, wait);
	return 0;
}

static int BiscuitOS_freeze(struct super_block *sb)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_unfreeze(struct super_block *sb)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_remount(struct super_block *sb, int *flags, char *data)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_show_options(struct seq_file *seq, struct dentry *root)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

/** Direct inode operation **/

static int BiscuitOS_create(struct inode *idr, struct dentry *dentry,
			umode_t mode, bool excl)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static struct dentry *BiscuitOS_lookup(struct inode *dir,
			struct dentry *dentry, unsigned int flags)
{
	printk("\n\n\n%s\n\n", __func__);
	return NULL;
}

static int BiscuitOS_link(struct dentry *old_dentry, struct inode *dir,
				struct dentry *dentry)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_unlink(struct inode *dir, struct dentry *dentry)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_symlink(struct inode *dir, struct dentry *dentry,
		const char *symname)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_mkdir(struct inode *dir, struct dentry *dentry,
					umode_t mode)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_rmdir(struct inode *dir, struct dentry *dentry)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_rename(struct inode *old_dir, struct dentry *old_dentry,
			    struct inode *new_dir, struct dentry *new_dentry,
			    unsigned int flags)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_tmpfile(struct inode *dir, struct dentry *dentry,
								umode_t mode)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

/** File inode operation **/

static int BiscuitOS_setattr(struct dentry *dentry, struct iattr *iattr)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_fiemap(struct inode *inode, 
		struct fiemap_extent_info *fieinfo, u64 start, u64 len)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

#define BiscuitOS_get_acl	NULL
#define BiscuitOS_set_acl	NULL

/** Address space operation **/

static int BiscuitOS_readpage(struct file *file, struct page *page)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_readpages(struct file *file, 
		struct address_space *mapping, struct list_head *pages,
		unsigned nr_pages)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_nobh_writepage(struct page *page,
			struct writeback_control *wbc)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_nobh_write_begin(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned flags, struct page **pagep, void **fsdata)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_write_end(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned copied, struct page *page, void *fsdata)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_write_begin(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned flags, struct page **pagep, void **fsdata)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_writepage(struct page *page,
					struct writeback_control *wbc)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static sector_t BiscuitOS_bmap(struct address_space *mapping, sector_t block)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_writepages(struct address_space *mapping,
					struct writeback_control *wbc)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static ssize_t BiscuitOS_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_dax_writepages(struct address_space *mapping,
			struct writeback_control *wbc)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

/** Direct inode operations **/

static int BiscuitOS_readdir(struct file *file, struct dir_context *ctx)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

/** File operations **/

static ssize_t BiscuitOS_file_read_iter(struct kiocb *iocb, 
						struct iov_iter *to)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static ssize_t BiscuitOS_file_write_iter(struct kiocb *iocb,
						struct iov_iter *from)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static long BiscuitOS_ioctl(struct file *filp, unsigned int cmd, 
							unsigned long arg)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_release_file(struct inode *inode, struct file *filp)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static int BiscuitOS_fsync(struct file *file, loff_t start, 
						loff_t end, int datasync)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

#define BiscuitOS_file_mmap	generic_file_mmap

/** Export operations **/
static struct dentry *BiscuitOS_fh_to_dentry(struct super_block *sb,
			struct fid *fid, int fh_len, int fh_type)
{
	printk("\n\n\n%s\n\n", __func__);
	return NULL;
}

static struct dentry *BiscuitOS_fh_to_parent(struct super_block *sb,
			struct fid *fid, int fh_len, int fh_type)
{
	printk("\n\n\n%s\n\n", __func__);
	return NULL;
}

static struct dentry *BiscuitOS_get_parent(struct dentry *child)
{
	printk("\n\n\n%s\n\n", __func__);
	return NULL;
}

static struct BiscuitOS_inode *BiscuitOS_get_inode(struct super_block *sb,
			ino_t ino, struct buffer_head **p)
{
	struct buffer_head *bh;
	unsigned long block_group;
	unsigned long block;
	unsigned long offset;
	struct BiscuitOS_group_desc *gdp;

	*p = NULL;
	if ((ino != BISCUITOS_ROOT_INO && ino < BISCUITOS_FIRST_INO(sb)) ||
	     ino > le32_to_cpu(BISCUITOS_SB(sb)->s_bs->s_inodes_count))
		goto Einval;

	block_group = (ino - 1) / BISCUITOS_INODES_PER_GROUP(sb);
	gdp = BiscuitOS_get_group_desc(sb, block_group, NULL);
	if (!gdp)
		goto Egdp;

	/*
	 * Figure out the offset within the block group inode table
	 */
	offset = ((ino - 1) % BISCUITOS_INODES_PER_GROUP(sb)) * 
						BISCUITOS_INODE_SIZE(sb);
	block = le32_to_cpu(gdp->bg_inode_table) +
			(offset >> BISCUITOS_BLOCK_SIZE_BITS(sb));
	if (!(bh = sb_bread(sb, block)))
		goto Eio;

	*p = bh;
	offset &= (BISCUITOS_BLOCK_SIZE(sb) - 1);
	return (struct BiscuitOS_inode *)(bh->b_data + offset);

Einval:
	printk("Bad inode number: %lu\n", (unsigned long)ino);
	return ERR_PTR(-EINVAL);
Eio:
	printk("Unable to read inode block - inode=%lu, block=%lu",
				(unsigned long)ino, block);
Egdp:
	return ERR_PTR(-EIO);
}

static void BiscuitOS_set_inode_flags(struct inode *inode)
{
	unsigned int flags = BISCUITOS_I(inode)->i_flags;

	inode->i_flags &= ~(S_SYNC | S_APPEND | S_IMMUTABLE | S_NOATIME |
				S_DIRSYNC | S_DAX);
	if (flags & BISCUITOS_SYNC_FL)
		inode->i_flags |= S_SYNC;
	if (flags & BISCUITOS_APPEND_FL)
		inode->i_flags |= S_APPEND;
	if (flags & BISCUITOS_IMMUTABLE_FL)
		inode->i_flags |= S_IMMUTABLE;
	if (flags & BISCUITOS_NOATIME_FL)
		inode->i_flags |= S_NOATIME;
	if (flags & BISCUITOS_DIRSYNC_FL)
		inode->i_flags |= S_DIRSYNC;
	if (test_opt(inode->i_sb, DAX) && S_ISREG(inode->i_mode))
		inode->i_flags |= S_DAX;
}

/*
 * Returns 1 if the passed-in block region is valid; 0 if some part overlaps
 * with filesystem metadata blocks.
 */
static int BiscuitOS_data_block_valid(struct BiscuitOS_sb_info *sbi,
		BiscuitOS_fsblk_t start_blk, unsigned int count)
{
	if ((start_blk <= le32_to_cpu(sbi->s_bs->s_first_data_block)) ||
	    (start_blk + count < start_blk) ||
	    (start_blk > le32_to_cpu(sbi->s_bs->s_blocks_count)))
		return 0;

	/* Ensure we do not step over superblock */
	if ((start_blk <= sbi->s_sb_block) &&
	    (start_blk + count >= sbi->s_sb_block))
		return 0;

	return 1;
}

static void BiscuitOS_set_file_ops(struct inode *inode)
{
	inode->i_op = &BiscuitOS_file_inode_operations;
	inode->i_fop = &BiscuitOS_file_operations;
	if (IS_DAX(inode))
		inode->i_mapping->a_ops = &BiscuitOS_dax_aops;
	else if (test_opt(inode->i_sb, NOBH))
		inode->i_mapping->a_ops = &BiscuitOS_nobh_aops;
	else
		inode->i_mapping->a_ops = &BiscuitOS_aops;
}

static inline int BiscuitOS_inode_is_fast_symlink(struct inode *inode)
{
	printk("\n\n\n%s\n\n", __func__);
	return 0;
}

static struct inode *BiscuitOS_iget(struct super_block *sb, unsigned long ino)
{
	struct BiscuitOS_inode_info *bi;
	struct buffer_head *bh;
	struct BiscuitOS_inode *raw_inode;
	struct inode *inode;
	long ret = -EIO;
	int n;
	uid_t i_uid;
	gid_t i_gid;

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	bi = BISCUITOS_I(inode);
	bi->i_block_alloc_info = NULL;

	raw_inode = BiscuitOS_get_inode(inode->i_sb, ino, &bh);
	if (IS_ERR(raw_inode)) {
		ret = PTR_ERR(raw_inode);
		goto bad_inode;
	}

	inode->i_mode = le16_to_cpu(raw_inode->i_mode);
	i_uid = (uid_t)le16_to_cpu(raw_inode->i_uid_low);
	i_gid = (gid_t)le16_to_cpu(raw_inode->i_gid_low);
	if (!(test_opt(inode->i_sb, NO_UID32))) {
		i_uid |= le16_to_cpu(raw_inode->i_uid_high) << 16;
		i_gid |= le16_to_cpu(raw_inode->i_gid_high) << 16;
	}
	i_uid_write(inode, i_uid);
	i_gid_write(inode, i_gid);
	set_nlink(inode, le16_to_cpu(raw_inode->i_links_count));
	inode->i_size = le32_to_cpu(raw_inode->i_size);
	inode->i_atime.tv_sec = (signed)le32_to_cpu(raw_inode->i_atime);
	inode->i_ctime.tv_sec = (signed)le32_to_cpu(raw_inode->i_ctime);
	inode->i_mtime.tv_sec = (signed)le32_to_cpu(raw_inode->i_mtime);
	inode->i_atime.tv_nsec = inode->i_mtime.tv_nsec =
				 inode->i_ctime.tv_nsec = 0;
	bi->i_dtime = le32_to_cpu(raw_inode->i_dtime);
	/* We now have engouh fields to check if the inode was active or not.
	 * This is needed because nfsd might try to access dead inodes
	 * the test is that same one that e2fsck uses
	 */
	if (inode->i_nlink == 0 && (inode->i_mode == 0 || bi->i_dtime)) {
		/* this inode is deleted */
		brelse(bh);
		ret = -ESTALE;
		goto bad_inode;
	}
	inode->i_blocks = le32_to_cpu(raw_inode->i_blocks);
	bi->i_flags = le32_to_cpu(raw_inode->i_flags);
	BiscuitOS_set_inode_flags(inode);
	bi->i_faddr = le32_to_cpu(raw_inode->i_faddr);
	bi->i_frag_no = raw_inode->i_frag;
	bi->i_frag_size = raw_inode->i_fsize;
	bi->i_file_acl = le32_to_cpu(raw_inode->i_file_acl);
	bi->i_dir_acl = 0;

	if (bi->i_file_acl &&
	  !BiscuitOS_data_block_valid(BISCUITOS_SB(sb), bi->i_file_acl , 1)) {
		printk("Bad extended attribute block %u\n", bi->i_file_acl);
		brelse(bh);
		ret = -EFSCORRUPTED;
		goto bad_inode;
	}

	if (S_ISREG(inode->i_mode))
		inode->i_size |= 
			((__u64)le32_to_cpu(raw_inode->i_size_high)) << 32;
	else
		bi->i_dir_acl = le32_to_cpu(raw_inode->i_dir_acl);
	if (i_size_read(inode) < 0) {
		ret = -EFSCORRUPTED;
		goto bad_inode;
	}
	bi->i_dtime = 0;
	inode->i_generation = le32_to_cpu(raw_inode->i_generation);
	bi->i_state = 0;
	bi->i_block_group = (ino - 1) / 
				BISCUITOS_INODES_PER_GROUP(inode->i_sb);
	bi->i_dir_start_lookup = 0;

	/*
	 * NOTE! The in-memory inode i_data array is in little-endian order
	 * even on big-endian machines: we do NOT byteswap the block numbers!
	 */
	for (n = 0; n < BISCUITOS_N_BLOCKS; n++)
		bi->i_data[n] = raw_inode->i_block[n];

	if (S_ISREG(inode->i_mode)) {
		BiscuitOS_set_file_ops(inode);
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &BiscuitOS_dir_inode_operations;
		inode->i_fop = &BiscuitOS_dir_operations;
		if (test_opt(inode->i_sb, NOBH))
			inode->i_mapping->a_ops = &BiscuitOS_nobh_aops;
		else
			inode->i_mapping->a_ops = &BiscuitOS_aops;
	} else if (S_ISLNK(inode->i_mode)) {
		if (BiscuitOS_inode_is_fast_symlink(inode)) {
			inode->i_link = (char *)bi->i_data;
			inode->i_op = &BiscuitOS_fast_symlink_inode_operations;
			nd_terminate_link(bi->i_data, inode->i_size,
						sizeof(bi->i_data) - 1);
		} else {
			inode->i_op = &BiscuitOS_symlink_inode_operations;
			inode_nohighmem(inode);
			if (test_opt(inode->i_sb, NOBH))
				inode->i_mapping->a_ops = &BiscuitOS_nobh_aops;
			else
				inode->i_mapping->a_ops = &BiscuitOS_aops;
		}
	} else {
		inode->i_op = &BiscuitOS_special_inode_operations;
		if (raw_inode->i_block[0])
			init_special_inode(inode, inode->i_mode,
			   old_decode_dev(le32_to_cpu(raw_inode->i_block[0])));
		else
			init_special_inode(inode, inode->i_mode,
			   new_decode_dev(le32_to_cpu(raw_inode->i_block[1])));
	}
	brelse(bh);
	unlock_new_inode(inode);
	return inode;

bad_inode:
	iget_failed(inode);
	return NULL;
}

static int BiscuitOS_setup_super(struct super_block *sb,
			struct BiscuitOS_super_block *bs, int read_only)
{
	int res = 0;
	struct BiscuitOS_sb_info *sbi = BISCUITOS_SB(sb);

	if (le32_to_cpu(bs->s_rev_level) > BISCUITOS_MAX_SUPP_REV) {
		printk("Error: revision level too high, forcing "
			"read-only mode\n");
		res = SB_RDONLY;
	}
	if (read_only)
		return res;
	if (!(sbi->s_mount_state & BISCUITOS_VALID_FS))
		printk("warning: mounting unchecked fs, running e2fsch is "
			"recommended\n");
	else if ((sbi->s_mount_state & BISCUITOS_ERROR_FS))
		printk("warning: mounting fs with errors, running e2fsck is "
			"recommended\n");
	else if ((__s16)le16_to_cpu(bs->s_max_mnt_count) >= 0 &&
		le16_to_cpu(bs->s_mnt_count) >=
		(unsigned short)(__s16)le16_to_cpu(bs->s_max_mnt_count))
		printk("warning: maximal mount count reached, running "
			"e2fsck is recommended\n");
	else if (le32_to_cpu(bs->s_checkinterval) &&
		(le32_to_cpu(bs->s_lastcheck) +
			le32_to_cpu(bs->s_checkinterval) <= 
			ktime_get_real_seconds()))
		printk("warning: checktime reached, running e2fsck is "
			"recommended\n");
	if (!le16_to_cpu(bs->s_max_mnt_count))
		bs->s_max_mnt_count = cpu_to_le16(BISCUITOS_DFL_MAX_MNT_COUNT);
	le16_add_cpu(&bs->s_mnt_count, 1);
	if (test_opt(sb, DEBUG))
		printk("%s, %s, bs=%lu, fs=%lu, gc=%lu, bgp=%lu, ipg=%lu, "
			"mo=%04lx]", BISCUITOS_VERSION, BISCUITOS_DATE,
			sb->s_blocksize, sbi->s_frag_size, sbi->s_groups_count,
			BISCUITOS_BLOCKS_PER_GROUP(sb),
			BISCUITOS_INODES_PER_GROUP(sb),
			sbi->s_mount_opt);
	return res;
}

static void BiscuitOS_write_super(struct super_block *sb)
{
	if (!sb_rdonly(sb))
		BiscuitOS_sync_fs(sb, 1);
}

static int BiscuitOS_fill_super(struct super_block *sb, void *data, int silent)
{
	struct dax_device *dax_dev = fs_dax_get_by_bdev(sb->s_bdev);
	struct buffer_head *bh;
	struct BiscuitOS_sb_info *sbi;
	struct BiscuitOS_super_block *bs;
	struct inode *root;
	unsigned long block;
	unsigned long sb_block = get_sb_block(&data);
	unsigned long logic_sb_block;
	unsigned long offset = 0;
	unsigned long def_mount_opts;
	long ret = -ENOMEM;
	int blocksize = BLOCK_SIZE;
	int db_count;
	int i, j;
	__le32 features;
	int err;
	struct BiscuitOS_mount_options opts;
	
	sbi = kzalloc(sizeof(*sbi), GFP_KERNEL);
	if (!sbi)
		goto failed;

	sbi->s_blockgroup_lock =
		kzalloc(sizeof(struct blockgroup_lock), GFP_KERNEL);
	if (!sbi->s_blockgroup_lock) {
		kfree(sbi);
		goto failed;
	}
	sb->s_fs_info = sbi;
	sbi->s_sb_block = sb_block;
	sbi->s_daxdev = dax_dev;

	spin_lock_init(&sbi->s_lock);
	ret = -EINVAL;

	/*
	 * See what the current blocksize for the device is, and
	 * use that as the blocksize. Otherwise (or if the blocksize
	 * is smaller than the default) use the default.
	 * This is important for devices that have a hardware
	 * sectorsize that is larger than the default.
	 */
	blocksize = sb_min_blocksize(sb, BLOCK_SIZE);
	if (!blocksize) {
		printk("Error: unable to set blocksize");
		goto failed_sbi;
	}
	
	/*
	 * If the superblock doesn't start on a hardware sector boundary,
	 * calculate the offset.
	 */
	if (blocksize != BLOCK_SIZE) {
		logic_sb_block = (sb_block * BLOCK_SIZE) / blocksize;
		offset = (sb_block * BLOCK_SIZE) % blocksize;
	} else {
		logic_sb_block = sb_block;
	}

	if (!(bh = sb_bread(sb, logic_sb_block))) {
		printk("Error: unable to read superblock");
		goto failed_sbi;
	}
	/*
	 * Note: s_es muts be initialized as soon as possible because
	 *	 some BiscuitOS-hfs macro-instructions depend on its value
	 */
	bs = (struct BiscuitOS_super_block *)(((char *)bh->b_data) + offset);
	sbi->s_bs = bs;
	sb->s_magic = le16_to_cpu(bs->s_magic);

	if (sb->s_magic != BISCUITOS_SUPER_MAGIC)
		goto cantfind_bs;

	opts.s_mount_opt = 0;
	/* Set defaults befor we parse the mount options */
	def_mount_opts = le32_to_cpu(bs->s_default_mount_opts);
	if (def_mount_opts & BISCUITOS_DEFM_DEBUG)
		set_opt(opts.s_mount_opt, DEBUG);
	if (def_mount_opts & BISCUITOS_DEFM_BSDGROUPS)
		set_opt(opts.s_mount_opt, GRPID);
	if (def_mount_opts & BISCUITOS_DEFM_UID16)
		set_opt(opts.s_mount_opt, NO_UID32);
	if (le16_to_cpu(sbi->s_bs->s_errors) == BISCUITOS_ERRORS_PANIC)
		set_opt(opts.s_mount_opt, ERRORS_PANIC);
	else if (le16_to_cpu(sbi->s_bs->s_errors) == BISCUITOS_ERRORS_CONTINUE)
		set_opt(opts.s_mount_opt, ERRORS_CONT);
	else
		set_opt(opts.s_mount_opt, ERRORS_RO);

	opts.s_resuid = make_kuid(&init_user_ns, le16_to_cpu(bs->s_def_resuid));
	opts.s_resgid = make_kgid(&init_user_ns, le16_to_cpu(bs->s_def_resgid));
	
	set_opt(opts.s_mount_opt, RESERVATION);

	if (!parse_options((char *) data, sb, &opts))
		goto failed_mount;

	sbi->s_mount_opt = opts.s_mount_opt;
	sbi->s_resuid = opts.s_resuid;
	sbi->s_resgid = opts.s_resgid;

	sb->s_flags = (sb->s_flags & ~SB_POSIXACL) |
		((BISCUITOS_SB(sb)->s_mount_opt & BISCUITOS_MOUNT_POSIX_ACL) ?
			SB_POSIXACL : 0);
	sb->s_iflags |= SB_I_CGROUPWB;

	if (le32_to_cpu(bs->s_rev_level) == BISCUITOS_GOOD_OLD_REV &&
		(BISCUITOS_HAS_COMPAT_FEATURE(sb, ~0U) ||
		 BISCUITOS_HAS_RO_COMPAT_FEATURE(sb, ~0U) ||
		 BISCUITOS_HAS_INCOMPAT_FEATURE(sb, ~0U)))
			printk("Warning: feature flags set on rev 0 fs, "
				"running e2fsck is recommended\n");
	/*
	 * Check feature flags regardless of the revision level, since we
	 * previously didn't change the revision level when setting the flags,
	 * so there is a chance incompat flags are set on a rev 0 filesystem.
	 */
	features = BISCUITOS_HAS_INCOMPAT_FEATURE(sb, 
					~BISCUITOS_FEATURE_INCOMPAT_SUPP);
	if (features) {
		printk("Error: couldn't mount because of "
			"unsupported optional features (%x)\n",
			le32_to_cpu(features));
		goto failed_mount;
	}
	if (!sb_rdonly(sb) && (features = BISCUITOS_HAS_RO_COMPAT_FEATURE(sb,
				~BISCUITOS_FEATURE_RO_COMPAT_SUPP))) {
		printk("error: couldn't mount ROWR because of "
			"unsupported optional features (%x)\n",
			le32_to_cpu(features));
		goto failed_mount;
	}

	blocksize = BLOCK_SIZE << le32_to_cpu(sbi->s_bs->s_log_block_size);

	if (sbi->s_mount_opt & BISCUITOS_MOUNT_DAX) {
		if (!bdev_dax_supported(sb->s_bdev, blocksize)) {
			printk("DAX unsupported by block device. "
				"Turuning off DAX.\n");
			sbi->s_mount_opt &= ~BISCUITOS_MOUNT_DAX;
		}
	}

	/* If the blocksize doesn't match, re-read the thing.. */
	if (sb->s_blocksize != blocksize) {
		brelse(bh);

		if (!sb_set_blocksize(sb, blocksize)) {
			printk("Error: bad blocksize %d\n", blocksize);
			goto failed_sbi;
		}

		logic_sb_block = (sb_block * BLOCK_SIZE) / blocksize;
		offset = (sb_block * BLOCK_SIZE) % blocksize;
		bh = sb_bread(sb, logic_sb_block);
		if (!bh) {
			printk("Error: could't read superblock on 2nd try\n");
			goto failed_sbi;
		}
		bs = (struct BiscuitOS_super_block *)(((char *)bh->b_data) +
					offset);
		sbi->s_bs = bs;
		if (bs->s_magic != cpu_to_le16(BISCUITOS_SUPER_MAGIC)) {
			printk("Error: magic mismatch\n");
			goto failed_mount;
		}
	}
	sb->s_maxbytes = BiscuitOS_max_size(sb->s_blocksize_bits);
	sb->s_max_links = BISCUITOS_LINK_MAX;

	if (le32_to_cpu(bs->s_rev_level) == BISCUITOS_GOOD_OLD_REV) {
		sbi->s_inode_size = BISCUITOS_GOOD_OLD_INODE_SIZE;
		sbi->s_first_ino = BISCUITOS_GOOD_OLD_FIRST_INO;
	} else {
		sbi->s_inode_size = le16_to_cpu(bs->s_inode_size);
		sbi->s_first_ino = le32_to_cpu(bs->s_first_ino);
		if ((sbi->s_inode_size < BISCUITOS_GOOD_OLD_INODE_SIZE) ||
			!is_power_of_2(sbi->s_inode_size) ||
			(sbi->s_inode_size > blocksize)) {
			printk("Error: unsupported inode size: %d\n",
					sbi->s_inode_size);
			goto failed_mount;
		}
	}

	sbi->s_frag_size = BISCUITOS_MIN_FRAG_SIZE <<
				le32_to_cpu(bs->s_log_frag_size);
	if (sbi->s_frag_size == 0)
		goto cantfind_bs;
	sbi->s_frags_per_block = sb->s_blocksize / sbi->s_frag_size;

	sbi->s_blocks_per_group = le32_to_cpu(bs->s_blocks_per_group);
	sbi->s_frags_per_group = le32_to_cpu(bs->s_frags_per_group);
	sbi->s_inodes_per_group = le32_to_cpu(bs->s_inodes_per_group);

	if (BISCUITOS_INODE_SIZE(sb) == 0)
		goto cantfind_bs;
	sbi->s_inodes_per_block = sb->s_blocksize / BISCUITOS_INODE_SIZE(sb);
	if (sbi->s_inodes_per_block == 0 || sbi->s_inodes_per_group == 0)
		goto cantfind_bs;
	sbi->s_itb_per_group = sbi->s_inodes_per_group /
					sbi->s_inodes_per_block;
	sbi->s_desc_per_block = sb->s_blocksize /
					sizeof(struct BiscuitOS_group_desc);
	sbi->s_sbh = bh;
	sbi->s_mount_state = le16_to_cpu(bs->s_state);
	sbi->s_addr_per_block_bits =
			ilog2(BISCUITOS_ADDR_PER_BLOCK(sb));
	sbi->s_desc_per_block_bits =
			ilog2(BISCUITOS_DESC_PER_BLOCK(sb));

	if (sb->s_magic != BISCUITOS_SUPER_MAGIC)
		goto cantfind_bs;

	if (sb->s_blocksize != bh->b_size) {
		if (!silent)
			printk("Error: unsupported blocksize.\n");
		goto failed_mount;
	}

	if (sb->s_blocksize != sbi->s_frag_size) {
		printk("Error: fragsize %lu != blocksize %lu"
			"(not support yet)", 
			sbi->s_frag_size, sb->s_blocksize);
		goto failed_mount;
	}

	if (sbi->s_blocks_per_group > sb->s_blocksize * 8) {
		printk("Error: #blocks per group too big: %lu\n",
				sbi->s_blocks_per_group);
		goto failed_mount;
	}
	if (sbi->s_inodes_per_group > sb->s_blocksize * 8) {
		printk("Error: #fragments per group too big: %lu\n",
				sbi->s_frags_per_group);
		goto failed_mount;
	}
	if (sbi->s_inodes_per_group > sb->s_blocksize * 8) {
		printk("Error: #inode per group too big: %lu\n",
				sbi->s_inodes_per_group);
		goto failed_mount;
	}

	if (BISCUITOS_BLOCKS_PER_GROUP(sb) == 0)
		goto cantfind_bs;
	sbi->s_groups_count = ((le32_to_cpu(bs->s_blocks_count) -
				le32_to_cpu(bs->s_first_data_block) - 1)
					/ BISCUITOS_BLOCKS_PER_GROUP(sb)) + 1;
	db_count = (sbi->s_groups_count + BISCUITOS_DESC_PER_BLOCK(sb) - 1) /
					BISCUITOS_DESC_PER_BLOCK(sb);
	sbi->s_group_desc = kmalloc_array(db_count,
					  sizeof(struct buffer_head *),
					  GFP_KERNEL);
	if (sbi->s_group_desc == NULL) {
		printk("Error: not enough memory\n");
		goto failed_mount;
	}
	bgl_lock_init(sbi->s_blockgroup_lock);
	sbi->s_debts = kcalloc(sbi->s_groups_count,
				sizeof(*sbi->s_debts),
				GFP_KERNEL);
	if (!sbi->s_debts) {
		printk("Error: not enough memory\n");
		goto failed_mount_group_desc;
	}
	for (i = 0; i < db_count; i++) {
		block = descriptor_loc(sb, logic_sb_block, i);
		sbi->s_group_desc[i] = sb_bread(sb, block);
		if (!sbi->s_group_desc[i]) {
			for (j = 0; j < i; j++)
				brelse(sbi->s_group_desc[j]);
			printk("Error: unable to read group descriptors\n");
			goto failed_mount_group_desc;
		}
	}
	if (!BiscuitOS_check_descriptors(sb)) {
		printk("Group descriptors corrupted.\n");
		goto failed_mount2;
	}
	sbi->s_gdb_count = db_count;
	get_random_bytes(&sbi->s_next_generation, sizeof(u32));
	spin_lock_init(&sbi->s_next_gen_lock);

	/* per filesystem reservation list head & lock */
	spin_lock_init(&sbi->s_rsv_window_lock);
	sbi->s_rsv_window_root = RB_ROOT;
	/*
	 * Add a single, static dummy reservation to the start of the
	 * reservation window list --- it gives us a placeholder for
	 * append-at-start-of-list which makes the allocation logic
	 * _much_ simple.
	 */
	sbi->s_rsv_window_head.rsv_start = 
					BISCUITOS_RESERVE_WINDOW_NOT_ALLOCATED;
	sbi->s_rsv_window_head.rsv_end =
					BISCUITOS_RESERVE_WINDOW_NOT_ALLOCATED;
	sbi->s_rsv_window_head.rsv_alloc_hit = 0;
	sbi->s_rsv_window_head.rsv_goal_size = 0;
	BiscuitOS_rsv_window_add(sb, &sbi->s_rsv_window_head);

	err = percpu_counter_init(&sbi->s_freeblocks_counter,
			BiscuitOS_count_free_blocks(sb), GFP_KERNEL);
	if (!err) {
		err = percpu_counter_init(&sbi->s_freeinodes_counter,
				BiscuitOS_count_free_inodes(sb), GFP_KERNEL);
	}
	if (!err) {
		err = percpu_counter_init(&sbi->s_dirs_counter,
				BiscuitOS_count_dirs(sb), GFP_KERNEL);
	}
	if (err) {
		printk("Error: insufficient memory\n");
		goto failed_mount3;
	}

	/* Set up enough so that it can read on inode */
	sb->s_op = &BiscuitOS_sops;
	sb->s_export_op = &BiscuitOS_export_ops;
	sb->s_xattr = BiscuitOS_xattr_handlers;

	root = BiscuitOS_iget(sb, BISCUITOS_ROOT_INO);
	if (IS_ERR(root)) {
		printk("Error: get root inode failed\n");
		ret = -ENOMEM;
		goto failed_mount3;
	} 
	if (BISCUITOS_HAS_COMPAT_FEATURE(sb, 
					BISCUITOS_FEATURE_COMPAT_HAS_JOURNAL))
		printk("Warning: mounting fs as BiscuitOS-hfs\n");
	if (BiscuitOS_setup_super(sb, bs, sb_rdonly(sb)))
		sb->s_flags |= SB_RDONLY;
	BiscuitOS_write_super(sb);
	return 0;

cantfind_bs:
	if (!silent)
		printk("Error: can't find an BiscuitOS-hfs filesystem on "
			"dev %s.", sb->s_id);
	goto failed_mount;

failed_mount3:
	BiscuitOS_xattr_destroy_cache(sbi->s_ea_block_cache);
	percpu_counter_destroy(&sbi->s_freeblocks_counter);
	percpu_counter_destroy(&sbi->s_freeinodes_counter);
	percpu_counter_destroy(&sbi->s_dirs_counter);
failed_mount2:
	for (i = 0; i < db_count; i++)
		brelse(sbi->s_group_desc[i]);
failed_mount_group_desc:
	kfree(sbi->s_group_desc);
	kfree(sbi->s_debts);
failed_mount:
	brelse(bh);
failed_sbi:
	sb->s_fs_info = NULL;
	kfree(sbi->s_blockgroup_lock);
	kfree(sbi);
failed:
	fs_put_dax(dax_dev);
	return ret;
}

static struct dentry *BiscuitOS_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, BiscuitOS_fill_super);
}

static const struct inode_operations BiscuitOS_special_inode_operations = {
	.setattr	= BiscuitOS_setattr,
	.get_acl	= BiscuitOS_get_acl,
	.set_acl	= BiscuitOS_set_acl,
};

static const struct inode_operations BiscuitOS_symlink_inode_operations = {
	.get_link	= page_get_link,
	.setattr	= BiscuitOS_setattr,
};

static const struct inode_operations BiscuitOS_fast_symlink_inode_operations = {
	.get_link	= simple_get_link,
	.setattr	= BiscuitOS_setattr,
};

static const struct file_operations BiscuitOS_dir_operations = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.iterate_shared	= BiscuitOS_readdir,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.fsync		= BiscuitOS_fsync,
};

static const struct inode_operations BiscuitOS_dir_inode_operations = {
	.create		= BiscuitOS_create,
	.lookup		= BiscuitOS_lookup,
	.link		= BiscuitOS_link,
	.unlink		= BiscuitOS_unlink,
	.symlink	= BiscuitOS_symlink,
	.mkdir		= BiscuitOS_mkdir,
	.rmdir		= BiscuitOS_rmdir,
	.rename		= BiscuitOS_rename,
	.setattr	= BiscuitOS_setattr,
	.get_acl	= BiscuitOS_get_acl,
	.set_acl	= BiscuitOS_set_acl,
	.tmpfile	= BiscuitOS_tmpfile,
};

static const struct address_space_operations BiscuitOS_aops = {
	.readpage	= BiscuitOS_readpage,
	.readpages	= BiscuitOS_readpages,
	.writepage	= BiscuitOS_writepage,
	.write_begin	= BiscuitOS_write_begin,
	.write_end	= BiscuitOS_write_end,
	.bmap		= BiscuitOS_bmap,
	.direct_IO	= BiscuitOS_direct_IO,
	.writepages	= BiscuitOS_writepages,
	.migratepage	= buffer_migrate_page,
	.is_partially_uptodate = block_is_partially_uptodate,
	.error_remove_page = generic_error_remove_page,
};

static const struct address_space_operations BiscuitOS_nobh_aops = {
	.readpage	= BiscuitOS_readpage,
	.readpages	= BiscuitOS_readpages,
	.writepage	= BiscuitOS_nobh_writepage,
	.write_begin	= BiscuitOS_nobh_write_begin,
	.write_end	= nobh_write_end,
	.bmap		= BiscuitOS_bmap,
	.direct_IO	= BiscuitOS_direct_IO,
	.writepages	= BiscuitOS_writepages,
	.migratepage	= buffer_migrate_page,
	.error_remove_page = generic_error_remove_page,
};

static const struct address_space_operations BiscuitOS_dax_aops = {
	.writepages	= BiscuitOS_dax_writepages,
	.direct_IO	= noop_direct_IO,
	.set_page_dirty	= noop_set_page_dirty,
	.invalidatepage	= noop_invalidatepage,
};

static const struct file_operations BiscuitOS_file_operations = {
	.llseek		= generic_file_llseek,
	.read_iter	= BiscuitOS_file_read_iter,
	.write_iter	= BiscuitOS_file_write_iter,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.mmap		= BiscuitOS_file_mmap,
	.open		= dquot_file_open,
	.release	= BiscuitOS_release_file,
	.fsync		= BiscuitOS_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.get_unmapped_area = thp_get_unmapped_area,
};

static const struct inode_operations BiscuitOS_file_inode_operations = {
	.setattr	= BiscuitOS_setattr,
	.get_acl	= BiscuitOS_get_acl,
	.set_acl	= BiscuitOS_set_acl,
	.fiemap		= BiscuitOS_fiemap,
};

static const struct export_operations BiscuitOS_export_ops = {
	.fh_to_dentry	= BiscuitOS_fh_to_dentry,
	.fh_to_parent	= BiscuitOS_fh_to_parent,
	.get_parent	= BiscuitOS_get_parent,
};

static const struct super_operations BiscuitOS_sops = {
	.alloc_inode	= BiscuitOS_alloc_inode,
	.destroy_inode	= BiscuitOS_destroy_inode,
	.write_inode	= BiscuitOS_write_inode,
	.evict_inode	= BiscuitOS_evict_inode,
	.put_super	= BiscuitOS_put_super,
	.sync_fs	= BiscuitOS_sync_fs,
	.freeze_fs	= BiscuitOS_freeze,
	.unfreeze_fs	= BiscuitOS_unfreeze,
	.statfs		= BiscuitOS_statfs,
	.remount_fs	= BiscuitOS_remount,
	.show_options	= BiscuitOS_show_options,
};

static struct file_system_type BiscuitOS_hfs_type = {
	.owner		= THIS_MODULE,
	.name		= "BiscuitOS-hfs",
	.mount		= BiscuitOS_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init BiscuitOS_init(void)
{
	int err;

	err = BiscuitOS_init_inodecache();
	if (err)
		return err;
	err = register_filesystem(&BiscuitOS_hfs_type);
	if (err)
		goto out;
	return 0;

out:
	BiscuitOS_destroy_inodecache();
	return err;
}
device_initcall(BiscuitOS_init);
