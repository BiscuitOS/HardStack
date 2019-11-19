#ifndef _BSFS_H_
#define _BSFS_H_

#include <linux/seq_file.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <linux/rbtree.h>
#include <linux/mutex.h>
#include <linux/xattr.h>

#define BS_DEACTIVATED_BIAS	(INT_MIN + 1)

enum bsfs_node_type {
	BSFS_DIR		= 0x0001,
	BSFS_FILE		= 0x0002,
	BSFS_LINK		= 0x0004,
};

#define BSFS_TYPE_MASK		0x000f
#define BSFS_FLAG_MASK		~BSFS_TYPE_MASK

enum bsfs_root_flag {
	BSFS_ROOT_CREATE_DEACTIVATED		= 0x0001,
	BSFS_ROOT_EXTRA_OPEN_PERM_CHECK		= 0x0002,
	BSFS_ROOT_SUPPORT_EXPORTOP		= 0x0004,
};

enum bsfs_node_flag {
	BSFS_ACTIVATED		= 0x0010,
	BSFS_NS			= 0x0020,
	BSFS_HAS_SEQ_SHOW	= 0x0040,
	BSFS_HAS_MMAP		= 0x0080,
	BSFS_LOCKDEP		= 0x0100,
	BSFS_SUICIDAL		= 0x0400,
	BSFS_SUICIDED		= 0x0800,
	BSFS_EMPTY_DIR		= 0x1000,
	BSFS_HAS_RELEASE	= 0x2000,
};

struct bsfs_open_file {
	struct bsfs_node	*kn;
	struct file		*file;
	struct seq_file		*seq_file;
	void			*priv;

	struct mutex		mutex;
	struct mutex		prealloc_mutex;
	int			event;
	struct list_head	list;
	char 			*prealloc_buf;

	size_t			atomic_write_len;
	bool			mmapped:1;
	bool			released:1;
	const struct vm_operations_struct *vm_ops;
};

struct bsfs_ops {
	int (*open)(struct bsfs_open_file *of);
	void (*release)(struct bsfs_open_file *of);

	int (*seq_show)(struct seq_file *sf, void *v);

	void *(*seq_start)(struct seq_file *sf, loff_t *ppos);
	void *(*seq_next)(struct seq_file *sf, void *v, loff_t *ppos);
	void (*seq_stop)(struct seq_file *sf, void *v);

	ssize_t (*read)(struct bsfs_open_file *of, char *buf, size_t bytes,
				loff_t off);
	size_t atomic_write_len;
	bool prealloc;
	ssize_t (*write)(struct bsfs_open_file *of, char *buf, size_t bytes,
				loff_t off);
	int (*mmap)(struct bsfs_open_file *of, struct vm_area_struct *vma);
};

struct bsfs_elem_dir {
	unsigned long 		subdirs;
	struct rb_root		children;

	struct bsfs_root	*root;
};

struct bsfs_elem_symlink {
	struct bsfs_node	*target_kn;
};

union bsfs_node_id {
	struct {
		u32	ino;
		u32	generation;
	};
	u64		id;
};

struct bsfs_elem_attr {
	const struct bsfs_ops *ops;
	struct bsfs_open_node	*open;
	loff_t			size;
	struct bsfs_node	*notify_next;
};

struct bsfs_node {
	atomic_t		count;
	atomic_t		active;

	struct bsfs_node	*parent;
	const char		*name;

	struct rb_node		rb;

	const void		*ns; /* namespace tag */
	unsigned int		hash; /* ns + name hash */
	union {
		struct bsfs_elem_dir		dir;
		struct bsfs_elem_symlink	symlink;
		struct bsfs_elem_attr		attr;
	};

	void			*priv;

	union bsfs_node_id	id;
	unsigned short		flags;
	umode_t			mode;
	struct bsfs_iattrs	*iattr;
};

struct bsfs_syscall_ops {
	int (*remount_fs)(struct bsfs_root *root, int *flags, char *data);
	int (*show_options)(struct seq_file *sf, struct bsfs_root *root);
	int (*mkdir)(struct bsfs_node *parent, const char *name,
			umode_t mode);
	int (*rmdir)(struct bsfs_node *kn);
	int (*rename)(struct bsfs_node *kn, struct bsfs_node *new_parent,
			const char *new_name);
	int (*show_path)(struct seq_file *sf, struct bsfs_node *kn,
			struct bsfs_root *root);
};

struct bsfs_root {
	/* published fields */
	struct bsfs_node	*kn;
	unsigned int		flags; /* KERNFS_ROOT_ flags */

	/* private fields, do not use outside kerfs proper */
	struct idr		ino_idr;
	u32			next_generation;
	struct bsfs_syscall_ops *syscall_ops;

	/* list of bsfs_super_info of this root, protected by bsfs_mutex */
	struct list_head	supers;

	wait_queue_head_t	deactivate_waitq;
};

struct bsfs_iattrs {
	struct iattr		ia_iattr;
	void 			*ia_secdata;
	u32			ia_secdata_len;

	struct simple_xattrs	xattrs;
};

static inline enum bsfs_node_type bsfs_type(struct bsfs_node *kn)
{
	return kn->flags & BSFS_TYPE_MASK;
}


extern struct bsfs_root *bsfs_create_root(struct bsfs_syscall_ops *scops,
				unsigned int flags, void *priv);
extern struct kmem_cache *bsfs_node_cache;
extern void bsfs_init(void);
extern int __bsfs_setattr(struct bsfs_node *kn, const struct iattr *iattr);
#endif
