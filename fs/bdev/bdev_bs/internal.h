#ifndef _BISCUITOS_BDEV_H
#define _BISCUITOS_BDEV_H

#include <linux/magic.h>

/* Bdev-fs MAGIC */
#define BDEVFS_MAGIC_BS		(BDEVFS_MAGIC + 0x10000000)

#define BS_DUP() printk("Expand..[%s][%s][%d]\n", __FILE__, __func__, __LINE__)

#endif
