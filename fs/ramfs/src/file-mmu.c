/*
 * Ramfs filesytem
 *
 * (C) 2020.03.04 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "internal.h"

static unsigned long ramfs_mmu_get_unmapped_area_bs(struct file *file,
			unsigned long addr, unsigned long len,
			unsigned long pgoff, unsigned long flags)
{
	BS_DUP();
	return 0;
}

const struct file_operations ramfs_file_operations_bs = {
	.read_iter		= generic_file_read_iter,
	.write_iter		= generic_file_write_iter,
	.mmap			= generic_file_mmap,
	.fsync			= noop_fsync,
	.splice_read		= generic_file_splice_read,
	.splice_write		= iter_file_splice_write,
	.llseek			= generic_file_llseek,
	.get_unmapped_area	= ramfs_mmu_get_unmapped_area_bs,
};

const struct inode_operations ramfs_file_inode_operations_bs = {
	.setattr		= simple_setattr,
	.getattr		= simple_getattr,
};
