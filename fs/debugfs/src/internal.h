#ifndef _BISCUITOS_DEBUGFS_H
#define _BISCUITOS_DEBUGFS_H

#include <linux/magic.h>

/* debugfs-bs MAGIC */
#define DEBUGFS_MAGIC_BS	(DEBUGFS_MAGIC + 0x10000000)

#define BS_DUP() printk("Expand.[%s][%s][%d]\n", __FILE__, __func__, __LINE__)

#endif
