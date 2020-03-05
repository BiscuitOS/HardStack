/*
 * Proc filesytem -- /proc/sys support
 *
 * (C) 2020.03.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysctl.h>
#include <linux/slab.h>

#include "internal.h"

static DEFINE_SPINLOCK(sysctl_lock_bs);

struct ctl_table sysctl_mount_point_bs[] = {
	{ }
};

static int insert_header_bs(struct ctl_dir *, struct ctl_table_header *);
static void drop_sysctl_table_bs(struct ctl_table_header *header);
static struct ctl_table_root sysctl_table_root_bs;

static bool is_empty_dir_bs(struct ctl_table_header *head)
{
	return head->ctl_table[0].child == sysctl_mount_point_bs;
}

static void set_empty_dir_bs(struct ctl_dir *dir)
{
	dir->header.ctl_table[0].child = sysctl_mount_point_bs;
}

static void clear_empty_dir_bs(struct ctl_dir *dir)
{
	dir->header.ctl_table[0].child = NULL;
}

static void sysctl_print_dir_bs(struct ctl_dir *dir)
{
	if (dir->header.parent)
		sysctl_print_dir_bs(dir->header.parent);
	pr_cont("%s/", dir->header.ctl_table[0].procname);
}

static struct dentry *proc_sys_lookup_bs(struct inode *dir,
			struct dentry *dentry, unsigned int flags)
{
	BS_DUP();
	return NULL;
}

static int proc_sys_permission_bs(struct inode *inode, int mask)
{
	BS_DUP();
	return 0;
}

static int proc_sys_setattr_bs(struct dentry *dentry, struct iattr *attr)
{
	BS_DUP();
	return 0;
}

/* called under sysctl_lock_bs */
static int use_table_bs(struct ctl_table_header *p)
{
	if (unlikely(p->unregistering))
		return 0;
	p->used++;
	return 1;
}

/* called under sysctl_lock_bs */
static void unuse_table_bs(struct ctl_table_header *p)
{
	if (!--p->used)
		if (unlikely(p->unregistering))
			complete(p->unregistering);
}

void proc_sys_evict_inode_bs(struct inode *inode, 
				struct ctl_table_header *head)
{
	spin_lock(&sysctl_lock_bs);
	hlist_del_init_rcu(&PROC_I_BS(inode)->sysctl_inodes);
	if (!--head->count)
		kfree_rcu(head, rcu);
	spin_unlock(&sysctl_lock_bs);
}

static void sysctl_head_finish_bs(struct ctl_table_header *head)
{
	if (!head)
		return;
	spin_lock(&sysctl_lock_bs);
	unuse_table_bs(head);
	spin_unlock(&sysctl_lock_bs);
}

static struct ctl_table_header *
sysctl_head_grab_bs(struct ctl_table_header *head)
{
	BUG_ON(!head);
	spin_lock(&sysctl_lock_bs);
	if (!use_table_bs(head))
		head = ERR_PTR(-ENOENT);
	spin_unlock(&sysctl_lock_bs);
	return head;
}

static struct ctl_table_header *grab_header_bs(struct inode *inode)
{
	struct ctl_table_header *head = PROC_I_BS(inode)->sysctl;

	if (!head)
		head = &sysctl_table_root_bs.default_set.dir.header;
	return sysctl_head_grab_bs(head);
}

static int proc_sys_getattr_bs(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct ctl_table_header *head = grab_header_bs(inode);
	struct ctl_table *table = PROC_I_BS(inode)->sysctl_entry;

	if (IS_ERR(head))
		return PTR_ERR(head);

	generic_fillattr(inode, stat);
	if (table)
		stat->mode = (stat->mode & S_IFMT) | table->mode;

	sysctl_head_finish_bs(head);
	return 0;
}

static int proc_sys_readdir_bs(struct file *file, struct dir_context *ctx)
{
	BS_DUP();
	return 0;
}

static const struct file_operations proc_sys_dir_file_operations_bs = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_sys_readdir_bs,
	.llseek		= generic_file_llseek,
};

static const struct inode_operations proc_sys_dir_operations_bs = {
	.lookup		= proc_sys_lookup_bs,
	.permission	= proc_sys_permission_bs,
	.setattr	= proc_sys_setattr_bs,
	.getattr	= proc_sys_getattr_bs,
};

int __init proc_sys_init_bs(void)
{
	struct proc_dir_entry *proc_sys_root;

	proc_sys_root = proc_mkdir_bs("sys", NULL);
	proc_sys_root->proc_iops = &proc_sys_dir_operations_bs;
	proc_sys_root->proc_fops = &proc_sys_dir_file_operations_bs;
	proc_sys_root->nlink = 0;

	return 0;
}

static struct ctl_table root_table_bs[] = {
	{
		.procname = "",
		.mode = S_IFDIR | S_IRUGO | S_IXUGO,
	},
	{ }
};

static struct ctl_table_root sysctl_table_root_bs = {
	.default_set.dir.header = {
		{
			{
				.count = 1,
				.nreg = 1,
				.ctl_table = root_table_bs
			}
		},
		.ctl_table_arg = root_table_bs,
		.root = &sysctl_table_root_bs,
		.set = &sysctl_table_root_bs.default_set,
	},
};

static int count_subheaders_bs(struct ctl_table *table)
{
	int has_files = 0;
	int nr_subheaders = 0;
	struct ctl_table *entry;

	/* special case: no directory and empty directory */
	if (!table || !table->procname)
		return 1;

	for (entry = table; entry->procname; entry++) {
		if (entry->child)
			nr_subheaders += count_subheaders_bs(entry->child);
		else
			has_files = 1;
	}
	return nr_subheaders + has_files;
}

static char *append_path_bs(const char *path, char *pos, const char *name)
{
	int namelen;
	namelen = strlen(name);
	if (((pos - path) + namelen + 2) >= PATH_MAX)
		return NULL;
	memcpy(pos, name, namelen);
	pos[namelen] = '/';
	pos[namelen + 1] = '\0';
	pos += namelen + 1;
	return pos;
}

static void init_header_bs(struct ctl_table_header *head,
		struct ctl_table_root *root, struct ctl_table_set *set,
		struct ctl_node *node, struct ctl_table *table)
{
	head->ctl_table = table;
	head->ctl_table_arg = table;
	head->used = 0;
	head->count = 1;
	head->nreg = 1;
	head->unregistering = NULL;
	head->root = root;
	head->set = set;
	head->parent = NULL;
	head->node = node;
	INIT_HLIST_HEAD(&head->inodes);
	if (node) {
		struct ctl_table *entry;
		for (entry = table; entry->procname; entry++, node++)
			node->header = head;
	}
}

static int sysctl_err_bs(const char *path, struct ctl_table *table, 
							char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	pr_err("sysctl table check failed: %s/%s %pV\n",
			path, table->procname, &vaf);
	va_end(args);
	return -EINVAL;
}

static int sysctl_check_table_bs(const char *path, struct ctl_table *table)
{
	BS_DUP();
	if (0)
		sysctl_err_bs(path, table, "Not a file");
	return 0;
}

static int namecmp_bs(const char *name1, int len1, const char *name2, int len2)
{
	int minlen;
	int cmp;

	minlen = len1;
	if (minlen > len2)
		minlen = len2;

	cmp = memcmp(name1, name2, minlen);
	if (cmp == 0)
		cmp = len1 - len2;
	return cmp;
}

/* Called under sysctl_lock_bs */
static struct ctl_table *find_entry_bs(struct ctl_table_header **phead,
		struct ctl_dir *dir, const char *name, int namelen)
{
	struct ctl_table_header *head;
	struct ctl_table *entry;
	struct rb_node *node = dir->root.rb_node;

	while (node) {
		struct ctl_node *ctl_node;
		const char *procname;
		int cmp;

		ctl_node = rb_entry(node, struct ctl_node, node);
		head = ctl_node->header;
		entry = &head->ctl_table[ctl_node - head->node];
		procname = entry->procname;

		cmp = namecmp_bs(name, namelen, procname, strlen(procname));
		if (cmp < 0)
			node = node->rb_left;
		else if (cmp > 0)
			node = node->rb_right;
		else {
			*phead = head;
			return entry;
		}
	}
	return NULL;
}

static struct ctl_dir *find_subdir_bs(struct ctl_dir *dir,
					const char *name, int namelen)
{
	struct ctl_table_header *head;
	struct ctl_table *entry;

	entry = find_entry_bs(&head, dir, name, namelen);
	if (!entry)
		return ERR_PTR(-ENOENT);
	if (!S_ISDIR(entry->mode))
		return ERR_PTR(-ENOTDIR);
	return container_of(head, struct ctl_dir, header);
}

static struct ctl_dir *new_dir_bs(struct ctl_table_set *set,
					const char *name, int namelen)
{
	struct ctl_table *table;
	struct ctl_dir *new;
	struct ctl_node *node;
	char *new_name;

	new = kzalloc(sizeof(*new) + sizeof(struct ctl_node) +
		      sizeof(struct ctl_table) * 2 + namelen + 1,
		      GFP_KERNEL);
	if (!new)
		return NULL;

	node = (struct ctl_node *)(new + 1);
	table = (struct ctl_table *)(node + 1);
	new_name = (char *)(table + 2);
	memcpy(new_name, name, namelen);
	new_name[namelen] = '\0';
	table[0].procname = new_name;
	table[0].mode = S_IFDIR | S_IRUGO | S_IXUGO;
	init_header_bs(&new->header, set->dir.header.root, set, node, table);

	return new;
}

static struct ctl_dir *xlate_dir_bs(struct ctl_table_set *set,
						struct ctl_dir *dir)
{
	struct ctl_dir *parent;
	const char *procname;

	if (!dir->header.parent)
		return &set->dir;
	parent = xlate_dir_bs(set, dir->header.parent);
	if (IS_ERR(parent))
		return parent;
	procname = dir->header.ctl_table[0].procname;
	return find_subdir_bs(parent, procname, strlen(procname));
}

static bool get_links_bs(struct ctl_dir *dir,
		struct ctl_table *table, struct ctl_table_root *link_root)
{
	struct ctl_table_header *head;
	struct ctl_table *entry, *link;

	/* Are there links available for every entry in table? */
	for (entry = table; entry->procname; entry++) {
		const char *procname = entry->procname;

		link = find_entry_bs(&head, dir, procname, strlen(procname));
		if (!link)
			return false;
		if (S_ISDIR(link->mode) && S_ISDIR(entry->mode))
			continue;
		if (S_ISLNK(link->mode) && (link->data == link_root))
			continue;
		return false;
	}

	/* The checks passed. Increase the registeration count on the links */
	for (entry = table; entry->procname; entry++) {
		const char *procname = entry->procname;

		link = find_entry_bs(&head, dir, procname, strlen(procname));
		head->nreg++;
	}
	return true;
}

static struct ctl_table_header *new_links_bs(struct ctl_dir *dir,
		struct ctl_table *table, struct ctl_table_root *link_root)
{
	struct ctl_table *link_table, *entry, *link;
	struct ctl_table_header *links;
	struct ctl_node *node;
	char *link_name;
	int nr_entries, name_bytes;

	name_bytes = 0;
	nr_entries = 0;

	for (entry = table; entry->procname; entry++) {
		nr_entries++;
		name_bytes += strlen(entry->procname) + 1;
	}

	links = kzalloc(sizeof(struct ctl_table_header) +
			sizeof(struct ctl_node *) * nr_entries + 
			sizeof(struct ctl_table) * (nr_entries + 1) + 
			name_bytes, GFP_KERNEL);

	if (!links)
		return NULL;

	node = (struct ctl_node *)(links + 1);
	link_table = (struct ctl_table *)(node + nr_entries);
	link_name = (char *)&link_table[nr_entries + 1];

	for (link = link_table, entry = table; entry->procname;
							link++, entry++) {
		int len = strlen(entry->procname) + 1;

		memcpy(link_name, entry->procname, len);
		link->procname = link_name;
		link->mode = S_IFLNK | S_IRWXUGO;
		link->data = link_root;
		link_name += len;
	}
	init_header_bs(links, dir->header.root, dir->header.set, node,
							link_table);
	links->nreg = nr_entries;

	return links;
}

static void put_links_bs(struct ctl_table_header *header)
{
	struct ctl_table_set *root_set = &sysctl_table_root_bs.default_set;
	struct ctl_table_root *root = header->root;
	struct ctl_dir *parent = header->parent;
	struct ctl_dir *core_parent;
	struct ctl_table *entry;

	if (header->set == root_set)
		return;

	core_parent = xlate_dir_bs(root_set, parent);
	if (IS_ERR(core_parent))
		return;

	for (entry = header->ctl_table; entry->procname; entry++) {
		struct ctl_table_header *link_head;
		struct ctl_table *link;
		const char *name = entry->procname;

		link = find_entry_bs(&link_head, core_parent, 
							name, strlen(name));
		if (link &&
		   ((S_ISDIR(link->mode) && S_ISDIR(entry->mode)) ||
		    (S_ISLNK(link->mode) && (link->data == root)))) {
			drop_sysctl_table_bs(link_head);
		} else {
			pr_err("sysctl link missing during unregister: ");
			sysctl_print_dir_bs(parent);
			pr_cont("/%s\n", name);
		}
	}
}

static void proc_sys_prune_dcache_bs(struct ctl_table_header *head)
{
	struct inode *inode;
	struct proc_inode *ei;
	struct hlist_node *node;
	struct super_block *sb;

	rcu_read_lock();
	for (;;) {
		node = hlist_first_rcu(&head->inodes);
		if (!node)
			break;
		ei = hlist_entry(node, struct proc_inode, sysctl_inodes);
		spin_lock(&sysctl_lock_bs);
		hlist_del_init_rcu(&ei->sysctl_inodes);
		spin_unlock(&sysctl_lock_bs);

		inode = &ei->vfs_inode;
		sb = inode->i_sb;
		if (!atomic_inc_not_zero(&sb->s_active))
			continue;
		inode = igrab(inode);
		rcu_read_unlock();
		if (unlikely(!inode)) {
			deactivate_super(sb);
			rcu_read_lock();
			continue;
		}

		d_prune_aliases(inode);
		iput(inode);
		deactivate_super(sb);

		rcu_read_lock();
	}
	rcu_read_unlock();
}

static void erase_entry_bs(struct ctl_table_header *head,
						struct ctl_table *entry)
{
	struct rb_node *node = &head->node[entry - head->ctl_table].node;

	rb_erase(node, &head->parent->root);
}

static void erase_header_bs(struct ctl_table_header *head)
{
	struct ctl_table *entry;

	for (entry = head->ctl_table; entry->procname; entry++)
		erase_entry_bs(head, entry);
}

/* called under sysctl_lock_bs, will reacquire if has to wait */
static void start_unregistering_bs(struct ctl_table_header *p)
{
	/*
	 * if p->used is 0, nobody will ever touch that entry again;
	 * we'll eliminate all paths to it before dropping sysctl_lock.
	 */
	if (unlikely(p->used)) {
		struct completion wait;
		init_completion(&wait);
		p->unregistering = &wait;
		spin_unlock(&sysctl_lock_bs);
		wait_for_completion(&wait);
	} else {
		/* anything non-NULL; we'll never dereference it */
		p->unregistering = ERR_PTR(-EINVAL);
		spin_unlock(&sysctl_lock_bs);
	}
	/*
	 * Prune dentries for unregistered sysctls: namespaced sysctls
	 * can have duplicate names and contaiminate dcache very badly.
	 */
	proc_sys_prune_dcache_bs(p);
	/*
	 * do not remove from the list until nobody holds it; walking the
	 * list in do_sysctl() relies on that.
	 */
	spin_lock(&sysctl_lock_bs);
	erase_header_bs(p);
}

static void drop_sysctl_table_bs(struct ctl_table_header *header)
{
	struct ctl_dir *parent = header->parent;

	if (--header->nreg)
		return;

	put_links_bs(header);
	start_unregistering_bs(header);
	if (!--header->count)
		kfree_rcu(header, rcu);

	if (parent)
		drop_sysctl_table_bs(&parent->header);
}

static int insert_links_bs(struct ctl_table_header *head)
{
	struct ctl_table_set *root_set = &sysctl_table_root_bs.default_set;
	struct ctl_dir *core_parent = NULL;
	struct ctl_table_header *links;
	int err;

	if (head->set == root_set)
		return 0;

	core_parent = xlate_dir_bs(root_set, head->parent);
	if (IS_ERR(core_parent))
		return 0;

	if (get_links_bs(core_parent, head->ctl_table, head->root))
		return 0;

	core_parent->header.nreg++;
	spin_unlock(&sysctl_lock_bs);

	links = new_links_bs(core_parent, head->ctl_table, head->root);

	spin_lock(&sysctl_lock_bs);
	err = -ENOMEM;
	if (!links)
		goto out;

	err = 0;
	if (get_links_bs(core_parent, head->ctl_table, head->root)) {
		kfree(links);
		goto out;
	}

	err = insert_header_bs(core_parent, links);
	if (err)
		kfree(links);

out:
	drop_sysctl_table_bs(&core_parent->header);
	return err;
}

static int insert_entry_bs(struct ctl_table_header *head, 
						struct ctl_table *entry)
{
	struct rb_node *node = &head->node[entry - head->ctl_table].node;
	struct rb_node **p = &head->parent->root.rb_node;
	struct rb_node *parent = NULL;
	const char *name = entry->procname;
	int namelen = strlen(name);

	while (*p) {
		struct ctl_table_header *parent_head;
		struct ctl_table *parent_entry;
		struct ctl_node *parent_node;
		const char *parent_name;
		int cmp;

		parent = *p;
		parent_node = rb_entry(parent, struct ctl_node, node);
		parent_head = parent_node->header;
		parent_entry = &parent_head->ctl_table[parent_node - 
							parent_head->node];
		parent_name = parent_entry->procname;

		cmp = namecmp_bs(name, namelen, parent_name, 
							strlen(parent_name));
		if (cmp < 0)
			p = &(*p)->rb_left;
		else if (cmp > 0)
			p = &(*p)->rb_right;
		else {
			pr_err("sysctl duplicate entry: ");
			sysctl_print_dir_bs(head->parent);
			pr_cont("/%s", entry->procname);
			return -EEXIST;
		}
	}

	rb_link_node(node, parent, p);
	rb_insert_color(node, &head->parent->root);

	return 0;
}

static int
insert_header_bs(struct ctl_dir *dir, struct ctl_table_header *header)
{
	struct ctl_table *entry;
	int err;

	/* Is this a permanently empty directory? */
	if (is_empty_dir_bs(&dir->header))
		return -EROFS;

	/* Am I creating a permanently empty directory */
	if (header->ctl_table == sysctl_mount_point_bs) {
		if (!RB_EMPTY_ROOT(&dir->root))
			return -EINVAL;
		set_empty_dir_bs(dir);
	}

	dir->header.nreg++;
	header->parent = dir;
	err = insert_links_bs(header);
	if (err)
		goto fail_links;
	for (entry = header->ctl_table; entry->procname; entry++) {
		err = insert_entry_bs(header, entry);
		if (err)
			goto fail;
	}
	return 0;

fail:
	erase_header_bs(header);
	put_links_bs(header);
fail_links:
	if (header->ctl_table == sysctl_mount_point_bs)
		clear_empty_dir_bs(dir);
	header->parent = NULL;
	drop_sysctl_table_bs(&dir->header);
	return err;
}

static struct ctl_dir *get_subdir_bs(struct ctl_dir *dir,
					const char *name, int namelen)
{
	struct ctl_table_set *set = dir->header.set;
	struct ctl_dir *subdir, *new = NULL;
	int err;

	spin_lock(&sysctl_lock_bs);
	subdir = find_subdir_bs(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	spin_unlock(&sysctl_lock_bs);
	new = new_dir_bs(set, name, namelen);
	spin_lock(&sysctl_lock_bs);
	subdir = ERR_PTR(-ENOMEM);
	if (!new)
		goto failed;

	/* Was the subdir added while we dropped the lock? */
	subdir = find_subdir_bs(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	/* Nope. Use the our freshly made directory entry. */
	err = insert_header_bs(dir, &new->header);
	subdir = ERR_PTR(err);
	if (err)
		goto failed;
	subdir = new;
found:
	subdir->header.nreg++;
failed:
	if (IS_ERR(subdir)) {
		pr_err("sysctl could not get directory: ");
		sysctl_print_dir_bs(dir);
		pr_cont("/%*.*s %ld\n",
			namelen, namelen, name, PTR_ERR(subdir));
	}
	drop_sysctl_table_bs(&dir->header);
	if (new)
		drop_sysctl_table_bs(&new->header);
	spin_unlock(&sysctl_lock_bs);
	return subdir;
}

struct ctl_table_header *__register_sysctl_table_bs(
	struct ctl_table_set *set, const char *path, struct ctl_table *table)
{
	struct ctl_table_root *root = set->dir.header.root;
	struct ctl_table_header *header;
	const char *name, *nextname;
	struct ctl_dir *dir;
	struct ctl_table *entry;
	struct ctl_node *node;
	int nr_entries = 0;

	for (entry = table; entry->procname; entry++)
		nr_entries++;

	header = kzalloc(sizeof(struct ctl_table_header) +
			 sizeof(struct ctl_node) * nr_entries, GFP_KERNEL);
	if (!header)
		return NULL;

	node = (struct ctl_node *)(header + 1);
	init_header_bs(header, root, set, node, table);
	if (sysctl_check_table_bs(path, table))
		goto fail;

	spin_lock(&sysctl_lock_bs);
	dir = &set->dir;
	/* Reference moved down the directory tree get_subdir */
	dir->header.nreg++;
	spin_unlock(&sysctl_lock_bs);

	/* Find the directory for the ctl_table */
	for (name = path; name; name = nextname) {
		int namelen;

		nextname = strchr(name, '/');
		if (nextname) {
			namelen = nextname - name;
			nextname++;
		} else {
			namelen = strlen(name);
		}
		if (namelen == 0)
			continue;

		dir = get_subdir_bs(dir, name, namelen);
		if (IS_ERR(dir))
			goto fail;
	}

	spin_lock(&sysctl_lock_bs);
	if (insert_header_bs(dir, header))
		goto fail_put_dir_locked;

	drop_sysctl_table_bs(&dir->header);
	spin_unlock(&sysctl_lock_bs);

	return header;

fail_put_dir_locked:
	drop_sysctl_table_bs(&dir->header);
	spin_unlock(&sysctl_lock_bs);
fail:
	kfree(header);
	dump_stack();
	return NULL;
}

static int register_leaf_sysctl_tables_bs(const char *path, char *pos,
	struct ctl_table_header ***subheader, struct ctl_table_set *set,
	struct ctl_table *table)
{
	struct ctl_table *ctl_table_arg = NULL;
	struct ctl_table *entry, *files;
	int nr_files = 0;
	int nr_dirs = 0;
	int err = -ENOMEM;

	for (entry = table; entry->procname; entry++) {
		if (entry->child)
			nr_dirs++;
		else
			nr_dirs++;
	}

	files = table;
	/* If there are mixed files and directories we need a new table */
	if (nr_dirs && nr_files) {
		struct ctl_table *new;

		files = kcalloc(nr_files + 1, sizeof(struct ctl_table),
							GFP_KERNEL);
		if (!files)
			goto out;

		ctl_table_arg = files;
		for (new = files, entry = table; entry->procname; entry++) {
			if (entry->child)
				continue;
			*new = *entry;
			new++;
		}
	}

	/* Register everything except a directory full of subdirectories */
	if (nr_files || !nr_dirs) {
		struct ctl_table_header *header;

		header = __register_sysctl_table_bs(set, path, files);
		if (!header) {
			kfree(ctl_table_arg);
			goto out;
		}

		/* Remember if we need to free the file table */
		header->ctl_table_arg = ctl_table_arg;
		**subheader = header;
		(*subheader)++;
	}

	/* Recurse into the subdirectories. */
	for (entry = table; entry->procname; entry++) {
		char *child_pos;

		if (!entry->child)
			continue;

		err = -ENAMETOOLONG;
		child_pos = append_path_bs(path, pos, entry->procname);
		if (!child_pos)
			goto out;

		err = register_leaf_sysctl_tables_bs(path, child_pos,
						subheader, set, entry->child);
		pos[0] = '\0';
		if (err)
			goto out;
	}
	err = 0;
out:
	/* On failure our caller will unregister all registered subheaders */
	return err;
}

void unregister_sysctl_table_bs(struct ctl_table_header *header)
{
	int nr_subheaders;

	might_sleep();

	if (header == NULL)
		return;

	nr_subheaders = count_subheaders_bs(header->ctl_table_arg);
	if (unlikely(nr_subheaders > 1)) {
		struct ctl_table_header **subheaders;
		int i;

		subheaders = (struct ctl_table_header **)(header + 1);
		for (i = nr_subheaders - 1; i >= 0; i--) {
			struct ctl_table_header *subh = subheaders[i];
			struct ctl_table *table = subh->ctl_table_arg;

			unregister_sysctl_table_bs(subh);
			kfree(table);
		}
		kfree(header);
		return;
	}

	spin_lock(&sysctl_lock_bs);
	drop_sysctl_table_bs(header);
	spin_unlock(&sysctl_lock_bs);
}

struct ctl_table_header *__register_sysctl_paths_bs(
		struct ctl_table_set *set, const struct ctl_path *path,
		struct ctl_table *table)
{
	struct ctl_table *ctl_table_arg = table;
	int nr_subheaders = count_subheaders_bs(table);
	struct ctl_table_header *header = NULL, **subheaders, **subheader;
	const struct ctl_path *component;
	char *new_path, *pos;

	pos = new_path = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!new_path)
		return NULL;

	pos[0] = '\0';
	for (component = path; component->procname; component++) {
		pos = append_path_bs(new_path, pos, table->procname);
		if (!pos)
			goto out;
	}
	while (table->procname && table->child && !table[1].procname) {
		pos = append_path_bs(new_path, pos, table->procname);
		if (!pos)
			goto out;
		table = table->child;
	}
	if (nr_subheaders == 1) {
		header = __register_sysctl_table_bs(set, new_path, table);
		if (header)
			header->ctl_table_arg = ctl_table_arg;
	} else {
		header = kzalloc(sizeof(*header) +
				 sizeof(*subheaders) * nr_subheaders, 
				 GFP_KERNEL);
		if (!header)
			goto out;

		subheaders = (struct ctl_table_header **)(header + 1);
		subheader = subheaders;
		header->ctl_table_arg = ctl_table_arg;

		if (register_leaf_sysctl_tables_bs(new_path, pos,
				&subheader, set, table))
			goto err_register_leaves;
	}

out:
	kfree(new_path);
	return header;

err_register_leaves:
	while (subheader > subheaders) {
		struct ctl_table_header *subh = *(--subheader);
		struct ctl_table *table = subh->ctl_table_arg;

		unregister_sysctl_table_bs(subh);
		kfree(table);
	}
	kfree(header);
	header = NULL;
	goto out;
}

struct ctl_table_header *register_sysctl_paths_bs(const struct ctl_path *path,
					struct ctl_table *table)
{
	return __register_sysctl_paths_bs(&sysctl_table_root_bs.default_set,
					path, table);
}

struct ctl_table_header *register_sysctl_table_bs(struct ctl_table *table)
{
	static const struct ctl_path null_path[] = { {} };

	return register_sysctl_paths_bs(null_path, table);
}
