#ifndef _BISCUITOS_RAMFS_BS_H
#define _BISCUITOS_RAMFS_BS_H

#include <uapi/linux/magic.h>

/* Ramfs_bs MAGIC */
#define RAMFS_MAGIC_BS	(RAMFS_MAGIC + 0x10000000)

extern const struct inode_operations ramfs_file_inode_operations_bs;
extern const struct file_operations ramfs_file_operations_bs;

#define BS_DUP() printk("Expand.%s %s + %d\n", __FILE__, __func__, __LINE__)

#endif
