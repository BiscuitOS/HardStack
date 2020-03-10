/*
 * minix-fs filesytem -- bitmap
 *
 * (C) 2020.03.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <uapi/linux/minix_fs.h>

#include "internal.h"

struct minix_inode *
minix_V1_raw_inode_bs(struct super_block *sb, ino_t ino,
						struct buffer_head **bh)
{
	int block;
	struct minix_sb_info *sbi = minix_sb_bs(sb);
	struct minix_inode *p;

	if (!ino || ino > sbi->s_ninodes) {
		printk("Bad inode number on dev %s: %ld is out of range\n",
				sb->s_id, (long)ino);
		return NULL;
	}
	ino--;
	block = 2 + sbi->s_imap_blocks + sbi->s_zmap_blocks +
			ino / MINIX_INODES_PER_BLOCK;
	*bh = sb_bread(sb, block);
	if (!*bh) {
		printk("Unable to read inode block\n");
		return NULL;
	}
	p = (void *)(*bh)->b_data;
	return p + ino % MINIX_INODES_PER_BLOCK;
}

struct minix2_inode *minix_V2_raw_inode_bs(struct super_block *sb,
					ino_t ino, struct buffer_head **bh)
{
	int block;
	struct minix_sb_info *sbi = minix_sb_bs(sb);
	struct minix2_inode *p;
	int minix2_inodes_per_block = sb->s_blocksize / 
						sizeof(struct minix2_inode);

	*bh = NULL;
	if (!ino || ino > sbi->s_ninodes) {
		printk("Bad inode number on dev %s: %ld is out of range\n",
			sb->s_id, (long)ino);
		return NULL;
	}
	ino--;
	block = 2 + sbi->s_imap_blocks + sbi->s_zmap_blocks +
			ino / minix2_inodes_per_block;
	*bh = sb_bread(sb, block);
	if (!*bh) {
		printk("Unable to read inode block\n");
		return NULL;
	}
	p = (void *)(*bh)->b_data;
	return p + ino % minix2_inodes_per_block;
}
