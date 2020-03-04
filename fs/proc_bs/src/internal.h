#ifndef _BISCUITOS_PROC_INTERNAL_H
#define _BISCUITOS_PROC_INTERNAL_H

#include <linux/fs.h>
#include <linux/proc_fs.h>

union proc_op {
	int (*proc_get_link)(struct dentry *, struct path *);
	int (*proc_show)(struct seq_file *m,
			struct pid_namespace *ns, struct pid *pid,
			struct task_struct *task);
};

struct proc_inode {
	struct pid *pid;
	unsigned int fd;
	union proc_op op;
	struct proc_dir_entry *pde;
	struct ctl_table_header *sysctl;
	struct ctl_table *sysctl_entry;
	struct hlist_node sysctl_inodes;
	const struct proc_ns_operations *ns_ops;
	struct inode vfs_inode;
} __randomize_layout;

/*
 * inode.c
 */
struct pde_opener {
	struct file *file;
	struct list_head lh;
	bool closing;
	struct completion *c;
} __randomize_layout;

struct proc_dir_entry {
	/*
	 * number of callers into module in progress;
	 * negative -> it's going away RSN
	 */
	atomic_t in_use;
	refcount_t refcnt;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	/* protects ->pde_openers and all struct pde_opener instances */
	spinlock_t pde_unload_lock;
	struct completion *pde_unload_completion;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	const struct dentry_operations *proc_dops;
	union {
		const struct seq_operations *seq_ops;
		int (*single_show)(struct seq_file *, void *);
	};
	proc_write_t write;
	void *data;
	unsigned int state_size;
	unsigned int low_ino;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	struct proc_dir_entry *parent;
	struct rb_root subdir;
	struct rb_node subdir_node;
	char *name;
	umode_t mode;
	u8 namelen;
	char inline_name[];
} __randomize_layout;

#define SIZEOF_PDE	(				\
	sizeof(struct proc_dir_entry) < 128 ? 128 :	\
	sizeof(struct proc_dir_entry) < 192 ? 192 :	\
	sizeof(struct proc_dir_entry) < 256 ? 256 :	\
	sizeof(struct proc_dir_entry) < 512 ? 512 :	\
	0)
#define SIZEOF_PDE_INLINE_NAME	(SIZEOF_PDE - sizeof(struct proc_dir_entry))

/* BiscuitOS MAGIC */
#define BISCUITOS_PROC_MAGIC		0x80000000

/*
 * Offset of the first process in the /proc root directory.
 */
#define FIRST_PROCESS_ENTRY		256

static inline bool is_empty_pde_bs(const struct proc_dir_entry *pde)
{
	return S_ISDIR(pde->mode) && !pde->proc_iops;
}

static inline struct proc_dir_entry *pde_get_bs(struct proc_dir_entry *pde)
{
	refcount_inc(&pde->refcnt);
	return pde;
}

static inline struct proc_inode *PROC_I_BS(const struct inode *inode)
{
	return container_of(inode, struct proc_inode, vfs_inode);
}

static inline struct proc_dir_entry *PDE_BS(const struct inode *inode)
{
	return PROC_I_BS(inode)->pde;
}

/* get the associated pid namespace for a file in procfs */
static inline struct pid_namespace *proc_pid_ns_bs(const struct inode *inode)
{
	return inode->i_sb->s_fs_info;
}

static inline struct pid *proc_pid_bs(const struct inode *inode)
{
	return PROC_I_BS(inode)->pid;
}

static inline struct task_struct *get_proc_task_bs(const struct inode *inode)
{
	return get_pid_task(proc_pid_bs(inode), PIDTYPE_PID);
}

/* Lookups */
typedef struct dentry *instantiate_t(struct dentry *,
				struct task_struct *, const void *);

extern struct proc_dir_entry proc_root_bs;
extern struct kmem_cache *proc_dir_entry_cache_bs;
extern void __init set_proc_pid_nlink_bs(void);
extern void __init proc_init_kmemcache_bs(void);
extern int proc_setattr_bs(struct dentry *dentry, struct iattr *attr);
extern const struct file_operations proc_fd_operations_bs;
extern const struct inode_operations proc_fd_inode_operations_bs;
extern void __init set_proc_pid_nlink_bs(void);
extern void __init proc_self_init_bs(void);
extern int proc_alloc_inum_bs(unsigned int *inum);
extern void __init proc_thread_self_init_bs(void);
extern struct proc_dir_entry *proc_symlink_bs(const char *name,
		struct proc_dir_entry *parent, const char *dest);
extern unsigned name_to_int_bs(const struct qstr *qstr);
extern const struct inode_operations proc_link_inode_operations_bs;
extern struct proc_dir_entry *proc_mkdir_bs(const char *name,
					struct proc_dir_entry *parent);
extern struct proc_dir_entry *proc_create_mount_point_bs(const char *name);
extern int __init proc_sys_init_bs(void);
extern int __init sysctl_init_bs(void);
extern struct ctl_table_header *
register_sysctl_table_bs(struct ctl_table *table);
extern int proc_fill_super_bs(struct super_block *s, void *data, int silent);
extern int proc_parse_options_bs(char *options, struct pid_namespace *pid);
extern int proc_remount_bs(struct super_block *sb, int *flags, char *data);
extern void pde_put_bs(struct proc_dir_entry *pde);
extern int proc_setup_self_bs(struct super_block *s);
extern int proc_setup_thread_self_bs(struct super_block *s);
extern int proc_readdir_bs(struct file *file, struct dir_context *ctx);
extern int proc_pid_readdir_bs(struct file *file, struct dir_context *ctx);
extern struct dentry *proc_pid_lookup_bs(struct inode *dir, 
			struct dentry *dentry, unsigned int flags);
extern struct inode *proc_get_inode_bs(struct super_block *sb,
						struct proc_dir_entry *de);
extern struct dentry *proc_lookup_bs(struct inode *dir, struct dentry *dentry,
					unsigned int flags);
extern void proc_sys_evict_inode_bs(struct inode *inode, 
					struct ctl_table_header *head);

#define BS_DUP()	printk("Expand.. %s %d\n", __FILE__, __LINE__)

#endif
