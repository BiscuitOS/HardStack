/*
 * Tmpfs/shmem filesytem
 *
 * (C) 2020.03.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/shmem_fs.h>
#include <linux/exportfs.h>
#include <linux/ctype.h>
#include <linux/module.h>

#include "internal.h"

static struct kmem_cache *shmem_inode_cachep_bs;

static void shmem_init_inode_bs(void *foo)
{
	struct shmem_inode_info *info = foo;

	inode_init_once(&info->vfs_inode);
}

static void shmem_init_inodecache_bs(void)
{
	shmem_inode_cachep_bs = kmem_cache_create("shmem_inode_cache_bs",
					sizeof(struct shmem_inode_info),
					0,
					SLAB_PANIC | SLAB_ACCOUNT,
					shmem_init_inode_bs);
}

static void shmem_destroy_inodecache_bs(void)
{
	kmem_cache_destroy(shmem_inode_cachep_bs);
}

static unsigned long shmem_default_max_blocks_bs(void)
{
	return CONFIG_BISCUITOS_TMPFS_BLOCKS;
}

static unsigned long shmem_default_max_inodes_bs(void)
{
	return CONFIG_BISCUITOS_TMPFS_INODES;
}

static struct dentry *shmem_get_parent_bs(struct dentry *child)
{
	BS_DUP();
	return NULL;
}

static int shmem_encode_fh_bs(struct inode *inode, __u32 *fh, int *len,
					struct inode *parent)
{
	BS_DUP();
	return 0;
}

static struct dentry *shmem_fh_to_dentry_bs(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	BS_DUP();
	return NULL;
}

static struct inode *shmem_alloc_inode_bs(struct super_block *sb)
{
	struct shmem_inode_info *info;

	info = kmem_cache_alloc(shmem_inode_cachep_bs, GFP_KERNEL);
	if (!info)
		return NULL;

	return &info->vfs_inode;
}

static void shmem_destroy_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static int shmem_statfs_bs(struct dentry *dentry, struct kstatfs *buf)
{
	BS_DUP();
	return 0;
}

static int shmem_remount_fs_bs(struct super_block *sb, int *flags, char *data)
{
	BS_DUP();
	return 0;
}

static int shmem_show_options_bs(struct seq_file *seq, struct dentry *root)
{
	BS_DUP();
	return 0;
}

static void shmem_evict_inode_bs(struct inode *inode)
{
	BS_DUP();
}

static void shmem_put_super_bs(struct super_block *sb)
{
	BS_DUP();
}

static const struct export_operations shmem_export_ops_bs = {
	.get_parent	= shmem_get_parent_bs,
	.encode_fh	= shmem_encode_fh_bs,
	.fh_to_dentry	= shmem_fh_to_dentry_bs,
};

static const struct super_operations shmem_ops_bs = {
	.alloc_inode	= shmem_alloc_inode_bs,
	.destroy_inode	= shmem_destroy_inode_bs,
	.statfs		= shmem_statfs_bs,
	.remount_fs	= shmem_remount_fs_bs,
	.show_options	= shmem_show_options_bs,
	.evict_inode	= shmem_evict_inode_bs,
	.drop_inode	= generic_delete_inode,
	.put_super	= shmem_put_super_bs,
};

static int shmem_parse_options_bs(char *options, struct shmem_sb_info *sbinfo,
			bool remount)
{
	char *this_char, *value, *rest;
	struct mempolicy *mpol = NULL;
	uid_t uid;
	gid_t gid;

	while (options != NULL) {
		this_char = options;
		for (;;) {
			/*
			 * NUL-terminate this option: unfirtunately,
			 * mount options form a comma-separated list,
			 * but mpol's nodelist may also contain commas.
			 */
			options = strchr(options, ',');
			if (options == NULL)
				break;
			options++;
			if (!isdigit(*options)) {
				options[-1] = '\0';
				break;
			}
		}
		if (!*this_char)
			continue;
		if ((value = strchr(this_char, '=')) != NULL) {
			*value++ = 0;
		} else {
			pr_err("tmpfs_bs: No value for mount option '%s'\n",
					this_char);
			goto error;
		}

		if (!strcmp(this_char, "size")) {
			unsigned long long size;

			size = memparse(value, &rest);
			if (*rest == '%') {
				size <<= PAGE_SHIFT;
				size *= totalram_pages();
				do_div(size, 100);
				rest++;
			}
			if (*rest)
				goto bad_val;
			sbinfo->max_blocks = 
					DIV_ROUND_UP(size, PAGE_SIZE);
		} else if (!strcmp(this_char, "nr_blocks")) {
			sbinfo->max_blocks = memparse(value, &rest);
			if (*rest)
				goto bad_val;
		} else if (!strcmp(this_char, "nr_inodes")) {
			sbinfo->max_inodes = memparse(value, &rest);
			if (*rest)
				goto bad_val;
		} else if (!strcmp(this_char, "mode")) {
			if (remount)
				continue;
			sbinfo->mode = simple_strtoul(value, &rest, 8) & 07777;
			if (*rest)
				goto bad_val;
		} else if (!strcmp(this_char, "uid")) {
			if (remount)
				continue;
			uid = simple_strtoul(value, &rest, 0);
			if (*rest)
				goto bad_val;
			sbinfo->uid = make_kuid(current_user_ns(), uid);
			if (!uid_valid(sbinfo->uid))
				goto bad_val;
		} else if (!strcmp(this_char, "gid")) {
			if (remount)
				continue;
			gid = simple_strtoul(value, &rest, 0);
			if (*rest)
				goto bad_val;
			sbinfo->gid = make_kgid(current_user_ns(), gid);
			if (!gid_valid(sbinfo->gid))
				goto bad_val;
		} else {
			pr_err("tmpfs_bs: Bad mount option %s\n", this_char);
			goto error;
		}
	}
	sbinfo->mpol = mpol;
	return 0;

bad_val:
	pr_err("tmpfs_bs: Bad value '%s' for mount option '%s'\n",
				value, this_char);
error:
	mpol_put(mpol);
	return 1;
}

int shmem_fill_super_bs(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	struct shmem_sb_info *sbinfo;
	int err = -ENOMEM;

	/* Round up to L1_CACHE_BYTES to resist false sharing */
	sbinfo = kzalloc(max((int)sizeof(struct shmem_sb_info),
					L1_CACHE_BYTES), GFP_KERNEL);
	if (!sbinfo)
		return -ENOMEM;

	sbinfo->mode = 0777 | S_ISVTX;
	sbinfo->uid = current_fsuid();
	sbinfo->gid = current_fsgid();
	sb->s_fs_info = sbinfo;

	/*
	 * Per default we only allow half of the physical ram per
	 * tmpfs instance, limiting inodes to one per page of lowmem;
	 * but the internal instance is left unlimited.
	 */
	if (!(sb->s_flags & SB_KERNMOUNT)) {
		sbinfo->max_blocks = shmem_default_max_blocks_bs();
		sbinfo->max_inodes = shmem_default_max_inodes_bs();
		if (shmem_parse_options_bs(data, sbinfo, false)) {
			err = -EINVAL;
			goto failed;
		}
	} else {
		sb->s_flags |= SB_NOUSER;
	}
	sb->s_export_op = &shmem_export_ops_bs;
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
	sb->s_magic = TMPFS_MAGIC_BS;
	sb->s_op = &shmem_ops_bs;
	sb->s_time_gran = 1;

	uuid_gen(&sb->s_uuid);

	inode = shmem_get_inode_bs(sb, NULL, 
				S_IFDIR | sbinfo->mode, 0, VM_NORESERVE);
	if (!inode)
		goto failed;
	inode->i_uid = sbinfo->uid;
	inode->i_gid = sbinfo->gid;
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		goto failed;
	return 0;

failed:
	shmem_put_super_bs(sb);
	return err;
}

static struct dentry *shmem_mount_bs(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, shmem_fill_super_bs);
}

static struct file_system_type shmem_fs_type_bs = {
	.owner		= THIS_MODULE,
	.name		= "tmpfs_bs",
	.mount		= shmem_mount_bs,
	.kill_sb	= kill_litter_super,
	.fs_flags	= FS_USERNS_MOUNT,
};

static int __init shmem_init_bs(void)
{
	int error;

	/* If rootfs called this, don't re-init */
	if (shmem_inode_cachep_bs)
		return 0;

	shmem_init_inodecache_bs();

	error = register_filesystem(&shmem_fs_type_bs);
	if (error) {
		pr_err("Could not register tmpfs\n");
		goto out;
	}

out:
	shmem_destroy_inodecache_bs();
	return error;
}
static void __exit shmem_exit_bs(void)
{
}

module_init(shmem_init_bs);
module_exit(shmem_exit_bs);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("tmpfs_bs filesystem");
