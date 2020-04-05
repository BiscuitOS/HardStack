/*
 * minix-fs filesytem -- inode
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <uapi/linux/minix_fs.h>
#include <linux/buffer_head.h>
#include <linux/module.h>

#include "internal.h"

static struct kmem_cache *minix_inode_cachep_bs;


static int minix_mknod_bs(struct inode *dir, struct dentry *dentry,
					umode_t mode, dev_t rdev)
{
	BS_DUP();
	return 0;
}

static int minix_create_bs(struct inode *dir, struct dentry *dentry,
					umode_t mode, bool excl)
{
	BS_DUP();
	return 0;
}

static struct dentry *minix_lookup_bs(struct inode *dir, 
		struct dentry *dentry, unsigned int flags)
{
	BS_DUP();
	return NULL;
}

static int minix_link_bs(struct dentry *old_dentry, struct inode *dir,
		struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int minix_unlink_bs(struct inode *dir, struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int minix_symlink_bs(struct inode *dir, struct dentry *dentry,
			const char *symname)
{
	BS_DUP();
	return 0;
}

static int minix_mkdir_bs(struct inode *dir, struct dentry *dentry,
							umode_t mode)
{
	BS_DUP();
	return 0;
}

static int minix_rmdir_bs(struct inode *dir, struct dentry *dentry)
{
	BS_DUP();
	return 0;
}

static int minix_rename_bs(struct inode *old_dir, struct dentry *old_dentry,
			struct inode *new_dir, struct dentry *new_dentry,
			unsigned int flags)
{
	BS_DUP();
	return 0;
}

int minix_getattr_bs(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int flags)
{
	BS_DUP();
	return 0;
}

static int minix_tmpfile_bs(struct inode *dir, struct dentry *dentry,
							umode_t mode)
{
	BS_DUP();
	return 0;
}

static int minix_readpage_bs(struct file *file, struct page *page)
{
	BS_DUP();
	return 0;
}

static int minix_writepage_bs(struct page *page, struct writeback_control *wbc)
{
	BS_DUP();
	return 0;
}

static int minix_write_begin_bs(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned flags, struct page **pagep, void **fsdata)
{
	BS_DUP();
	return 0;
}

static sector_t minix_bmap_bs(struct address_space *mapping, sector_t block)
{
	BS_DUP();
	return 0;
}

/*
 * directories can handle most operations...
 */
const struct inode_operations minix_dir_inode_operations_bs = {
	.create		= minix_create_bs,
	.lookup		= minix_lookup_bs,
	.link		= minix_link_bs,
	.unlink		= minix_unlink_bs,
	.symlink	= minix_symlink_bs,
	.mkdir		= minix_mkdir_bs,
	.rmdir		= minix_rmdir_bs,
	.mknod		= minix_mknod_bs,
	.rename		= minix_rename_bs,
	.getattr	= minix_getattr_bs,
	.tmpfile	= minix_tmpfile_bs,
};

static const struct address_space_operations minix_aops_bs = {
	.readpage	= minix_readpage_bs,
	.writepage	= minix_writepage_bs,
	.write_begin	= minix_write_begin_bs,
	.write_end	= generic_write_end,
	.bmap		= minix_bmap_bs,
};

static void init_once_bs(void *foo)
{
	struct minix_inode_info *ei = (struct minix_inode_info *)foo;

	inode_init_once(&ei->vfs_inode);
}

static void destroy_inodecache_bs(void)
{
	rcu_barrier();
	kmem_cache_destroy(minix_inode_cachep_bs);
}

static int __init init_inodecache_bs(void)
{
	minix_inode_cachep_bs = kmem_cache_create("minix_inode_cache_bs",
					sizeof(struct minix_inode_info),
					0,
					(SLAB_RECLAIM_ACCOUNT |
					SLAB_MEM_SPREAD | SLAB_ACCOUNT),
					init_once_bs);
	if (minix_inode_cachep_bs == NULL)
		return -ENOMEM;
	return 0;
}

void minix_set_inode_bs(struct inode *inode, dev_t rdev)
{
	if (S_ISREG(inode->i_mode)) {
		BS_DUP();
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &minix_dir_inode_operations_bs;
		inode->i_fop = &minix_dir_operations_bs;
		inode->i_mapping->a_ops = &minix_aops_bs;
	} else if (S_ISLNK(inode->i_mode)) {
		BS_DUP();
	} else
		BS_DUP();
}

/*
 * The minix V1 function to read an inode.
 */
static struct inode *V1_minix_iget_bs(struct inode *inode)
{
	struct buffer_head *bh;
	struct minix_inode *raw_inode;
	struct minix_inode_info *minix_inode = minix_i_bs(inode);
	int i;

	raw_inode = minix_V1_raw_inode_bs(inode->i_sb, inode->i_ino, &bh);
	if (!raw_inode) {
		iget_failed(inode);
		return ERR_PTR(-EIO);
	}
	inode->i_mode = raw_inode->i_mode;
	i_uid_write(inode, raw_inode->i_uid);
	i_gid_write(inode, raw_inode->i_gid);
	set_nlink(inode, raw_inode->i_size);
	inode->i_size = raw_inode->i_size;
	inode->i_mtime.tv_sec = inode->i_atime.tv_sec =
				inode->i_ctime.tv_sec = raw_inode->i_time;
	inode->i_mtime.tv_nsec = 0;
	inode->i_atime.tv_nsec = 0;
	inode->i_ctime.tv_nsec = 0;
	inode->i_blocks = 0;
	for (i = 0; i < 9; i++)
		minix_inode->u.i1_data[i] = raw_inode->i_zone[i];
	minix_set_inode_bs(inode, old_decode_dev(raw_inode->i_zone[0]));
	brelse(bh);
	unlock_new_inode(inode);
	return inode;
}

/*
 * The minix V2 function to read an inode.
 */
static struct inode *V2_minix_iget_bs(struct inode *inode)
{
	struct buffer_head *bh;
	struct minix2_inode *raw_inode;
	struct minix_inode_info *minix_inode = minix_i_bs(inode);
	int i;

	raw_inode = minix_V2_raw_inode_bs(inode->i_sb, inode->i_ino, &bh);
	if (!raw_inode) {
		iget_failed(inode);
		return ERR_PTR(-EIO);
	}
	inode->i_mode = raw_inode->i_mode;
	i_uid_write(inode, raw_inode->i_uid);
	i_gid_write(inode, raw_inode->i_gid);
	set_nlink(inode, raw_inode->i_nlinks);
	inode->i_size = raw_inode->i_size;
	inode->i_mtime.tv_sec = raw_inode->i_mtime;
	inode->i_atime.tv_sec = raw_inode->i_atime;
	inode->i_ctime.tv_sec = raw_inode->i_ctime;
	inode->i_mtime.tv_nsec = 0;
	inode->i_atime.tv_nsec = 0;
	inode->i_ctime.tv_nsec = 0;
	inode->i_blocks = 0;
	for (i = 0; i < 10; i++)
		minix_inode->u.i2_data[i] = raw_inode->i_zone[i];
	minix_set_inode_bs(inode, old_decode_dev(raw_inode->i_zone[0]));
	brelse(bh);
	unlock_new_inode(inode);
	return inode;
}

/*
 * The global function to read an inode.
 */
struct inode *minix_iget_bs(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	if (INODE_VERSION(inode) == MINIX_V1_BS)
		return V1_minix_iget_bs(inode);
	else
		return V2_minix_iget_bs(inode);
}

static struct inode *minix_alloc_inode_bs(struct super_block *sb)
{
	struct minix_inode_info *ei;

	ei = kmem_cache_alloc(minix_inode_cachep_bs, GFP_KERNEL);
	if (!ei)
		return NULL;
	return &ei->vfs_inode;
}

static void minix_destroy_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static int minix_write_inode_bs(struct inode *inode, 
					struct writeback_control *wbc)
{
	BS_DUP();
	return 0;
}

static void minix_evict_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static void minix_put_super_bs(struct super_block *sb)
{
	BS_DUP();
}

static int minix_statfs_bs(struct dentry *dentry, struct kstatfs *buf)
{
	BS_DUP();
	return 0;
}

static int minix_remount_bs(struct super_block *sb, int *flags, char *data)
{
	BS_DUP();
	return 0;
}

static const struct super_operations minix_sops_bs = {
	.alloc_inode	= minix_alloc_inode_bs,
	.destroy_inode	= minix_destroy_inode_bs,
	.write_inode	= minix_write_inode_bs,
	.evict_inode	= minix_evict_inode_bs,
	.put_super	= minix_put_super_bs,
	.statfs		= minix_statfs_bs,
	.remount_fs	= minix_remount_bs,
};

static int minix_fill_super_bs(struct super_block *s, void *data, int silent)
{
	struct buffer_head *bh;
	struct buffer_head **map;
	struct minix_super_block *ms;
	unsigned long i, block;
	struct inode *root_inode;
	struct minix_sb_info *sbi;
	int ret = -EINVAL;

	sbi = kzalloc(sizeof(struct minix_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;
	s->s_fs_info = sbi;

	BUILD_BUG_ON(32 != sizeof(struct minix_inode));
	BUILD_BUG_ON(64 != sizeof(struct minix2_inode));

	if (!sb_set_blocksize(s, BLOCK_SIZE))
		goto out_bad_hblock;

	if (!(bh = sb_bread(s, 1)))
		goto out_bad_sb;

	ms = (struct minix_super_block *)bh->b_data;
	sbi->s_ms = ms;
	sbi->s_sbh = bh;
	sbi->s_mount_state = ms->s_state;
	sbi->s_ninodes = ms->s_ninodes;
	sbi->s_nzones = ms->s_nzones;
	sbi->s_imap_blocks = ms->s_imap_blocks;
	sbi->s_zmap_blocks = ms->s_zmap_blocks;
	sbi->s_firstdatazone = ms->s_firstdatazone;
	sbi->s_log_zone_size = ms->s_log_zone_size;
	sbi->s_max_size = ms->s_max_size;
	s->s_magic = ms->s_magic;
	if (s->s_magic == MINIX_SUPER_MAGIC) {
		sbi->s_version = MINIX_V1_BS;
		sbi->s_dirsize = 16;
		sbi->s_namelen = 14;
		s->s_max_links = MINIX_LINK_MAX;
	} else if (s->s_magic == MINIX_SUPER_MAGIC2) {
		sbi->s_version = MINIX_V1_BS;
		sbi->s_dirsize = 32;
		sbi->s_namelen = 30;
		s->s_max_links = MINIX_LINK_MAX;
	} else if (s->s_magic == MINIX2_SUPER_MAGIC) {
		sbi->s_version = MINIX_V2_BS;
		sbi->s_nzones = ms->s_zones;
		sbi->s_dirsize = 32;
		sbi->s_namelen = 30;
		s->s_max_links = MINIX2_LINK_MAX;
	} else if (*(__u16 *)(bh->b_data + 24) == MINIX3_SUPER_MAGIC) {
		BS_DUP();
	} else
		goto out_no_fs;

	/*
	 * Allocate the buffer map to keep the superblock small.
	 */
	if (sbi->s_imap_blocks == 0 || sbi->s_zmap_blocks == 0)
		goto out_illegal_sb;
	i = (sbi->s_imap_blocks + sbi->s_zmap_blocks) * sizeof(bh);
	map = kzalloc(i, GFP_KERNEL);
	if (!map)
		goto out_no_map;
	sbi->s_imap = &map[0];
	sbi->s_zmap = &map[sbi->s_imap_blocks];

	block = 2;
	for (i = 0; i < sbi->s_imap_blocks; i++) {
		if (!(sbi->s_imap[i] = sb_bread(s, block)))
			goto out_no_bitmap;
		block++;
	}

	for (i = 0; i < sbi->s_zmap_blocks; i++) {
		if (!(sbi->s_zmap[i] = sb_bread(s, block)))
			goto out_no_bitmap;
		block++;
	}

	minix_set_bit_bs(0, sbi->s_imap[0]->b_data);
	minix_set_bit_bs(0, sbi->s_zmap[0]->b_data);

	block = minix_blocks_needed_bs(sbi->s_ninodes, s->s_blocksize);
	if (sbi->s_imap_blocks < block) {
		printk("MINIX-fs_bs: file system does not have enough "
			"imap blocks allocated. Refusing to mount.\n");
		goto out_no_bitmap;
	}

	block = minix_blocks_needed_bs(
			(sbi->s_nzones - sbi->s_firstdatazone + 1),
			s->s_blocksize);
	if (sbi->s_zmap_blocks < block) {
		printk("MINIX-fs_bs: file system does not have enough "
			"zmap blocks allocated. Refusing to mount.\n");
		goto out_no_bitmap;
	}

	/* setup up enough so that it can read an inode */
	s->s_op = &minix_sops_bs;
	root_inode = minix_iget_bs(s, MINIX_ROOT_INO);
	if (IS_ERR(root_inode)) {
		ret = PTR_ERR(root_inode);
		goto out_no_root;
	}

	ret = -ENOMEM;
	s->s_root = d_make_root(root_inode);
	if (!s->s_root)
		goto out_no_root;

	if (!sb_rdonly(s)) {
		if (sbi->s_version != MINIX_V3_BS)
			ms->s_state &= ~MINIX_VALID_FS;
		mark_buffer_dirty(bh);
	}
	if (!(sbi->s_mount_state & MINIX_VALID_FS))
		printk("MINIX-fs_bs: mounting unchecked file system, "
			"running fsck is recommended\n");
	else if (sbi->s_mount_state & MINIX_ERROR_FS)
		printk("MINIX-fs_bs: mounting file system with errors, "
			"running fsck is recommeded\n");
	return 0;

out_no_root:
	if (!silent)
		printk("MINIX-fs_bs: get root inode failed\n");
	goto out_freemap;

out_no_bitmap:
	printk("MINIX-fs: bad superblock or unable to read bitmaps\n");
out_freemap:
	for (i = 0; i < sbi->s_imap_blocks; i++)
		brelse(sbi->s_imap[i]);
	for (i = 0; i < sbi->s_zmap_blocks; i++)
		brelse(sbi->s_zmap[i]);
	kfree(sbi->s_imap);
	goto out_release;

out_no_map:
	ret = -ENOMEM;
	if (!silent)
		printk("MINIX-fs_bs: can't allocate map\n");
	goto out_release;

out_illegal_sb:
	if (!silent)
		printk("MINIX-fs_bs: bad superblock\n");
	goto out_release;

out_no_fs:
	if (!silent)
		printk("VFS: Can't find a Minix filesystem V1 | V2 | V3 "
			"on device %s.\n", s->s_id);

out_release:
	brelse(bh);
	goto out;

out_bad_hblock:
	printk("MINIX-fs_bs: blocksize too small for device\n");
	goto out;

out_bad_sb:
	printk("MINIX-fs_bs: unable to read superblock\n");
out:
	s->s_fs_info = NULL;
	kfree(sbi);
	return ret;
}

static struct dentry *minix_mount_bs(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, minix_fill_super_bs);
}

static struct file_system_type minix_fs_type_bs = {
	.owner		= THIS_MODULE,
	.name		= "minix_bs",
	.mount		= minix_mount_bs,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init init_minix_fs_bs(void)
{
	int err = init_inodecache_bs();

	if (err)
		goto out1;
	err = register_filesystem(&minix_fs_type_bs);
	if (err)
		goto out;
	return 0;

out:
	destroy_inodecache_bs();
out1:
	return err;
}

static void __exit exit_minix_fs_bs(void)
{
	unregister_filesystem(&minix_fs_type_bs);
}

module_init(init_minix_fs_bs);
module_exit(exit_minix_fs_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("minix_bs filesystem");
