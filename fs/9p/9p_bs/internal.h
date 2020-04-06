#ifndef _BISCUITOS_V9PS_H
#define _BISCUITOS_V9PS_H

#include <net/9p/9p.h>
#include <linux/backing-dev.h>

struct v9fs_inode {
	struct p9_qid qid;
	unsigned int cache_validity;
	struct p9_fid *writeback_fid;
	struct mutex v_mutex;
	struct inode vfs_inode;
};

#define BS_DUP()	printk("Expand.[%s-%d]\n", __func__, __LINE__)
extern struct file_system_type v9fs_fs_type_bs;

#endif
