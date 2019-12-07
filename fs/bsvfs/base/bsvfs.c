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

/* fs maigc */
#define BISCUITOS_FS_MAGIC	0x911016

/* Pretend that each entry is of this size in directory's i_size */
#define BOGO_DIRENT_SIZE	20

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

/* inode cache */
static struct kmem_cache *BiscuitOS_inode_cachep;
/* mount pointer */
static struct vfsmount *BiscuitOS_mnt;

/* head list */
static const struct inode_operations BiscuitOS_special_inode_operations;
static const struct address_space_operations BiscuitOS_aops;
static const struct inode_operations BiscuitOS_inode_operations;
static const struct file_operations BiscuitOS_file_operations;
static const struct inode_operations BiscuitOS_dir_inode_operations;
static const struct super_operations BiscuitOS_ops;
static const struct export_operations BiscuitOS_export_ops;

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

static void BiscuitOS_destroy_inode(struct inode *inode)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
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
	printk("\n\n\n\n%s\n\n\n", __func__);
}

static int BiscuitOS_getattr(const struct path *path, struct kstat *stat,
				u32 request_mask, unsigned int query_flags)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_setattr(struct dentry *dentry, struct iattr *attr)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
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

static int BiscuitOS_write_begin(struct file *file, 
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned flags, struct page **page, void **fsdata)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_write_end(struct file *file,
		struct address_space *mapping, loff_t pos, unsigned len,
		unsigned copied, struct page *page, void *fsdata)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
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
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
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

static int BiscuitOS_create(struct inode *dir, struct dentry *dentry,
						umode_t mdoe, bool excl)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
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
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_symlink(struct inode *dir, struct dentry *dentry,
					const char *synname)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_mkdir(struct inode *dir, 
				struct dentry *dentry, umode_t mode)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_rmdir(struct inode *dir, struct dentry *dentry)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

/* File creation. Allocate an inode, and we're done.. */
static int BiscuitOS_mknod(struct inode *dir, struct dentry *dentry,
					umode_t mode, dev_t dev)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
	return 0;
}

static int BiscuitOS_rename(struct inode *old_dir, struct dentry *old_dentry,
	struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	printk("\n\n\n\n%s\n\n\n", __func__);
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

static struct dentry *BiscuitOS_mount(struct file_system_type *fs_type,
			int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, BiscuitOS_fill_super);
}

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
