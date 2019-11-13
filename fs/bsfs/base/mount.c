#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/kernfs.h>
#include <linux/magic.h>
#include <linux/kobject.h>

#define BSVFS_MAGIC	0x911016

static struct kernfs_root *bsvfs_root;
struct kernfs_node *bsvfs_root_kn;

static struct dentry *bsvfs_mount(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	struct dentry *root;
	void *ns;
	bool new_sb = false;

	if (!(flags & SB_KERNMOUNT)) {
		if (!kobj_ns_current_may_mount(KOBJ_NS_TYPE_NET))
			return ERR_PTR(-EPERM);
	}

	ns = kobj_ns_grab_current(KOBJ_NS_TYPE_NET);
	root = kernfs_mount_ns(fs_type, flags, bsvfs_root,
				BSVFS_MAGIC, &new_sb, ns);
	if (!new_sb)
		kobj_ns_drop(KOBJ_NS_TYPE_NET, ns);
	else if (!IS_ERR(root))
		root->d_sb->s_iflags |= SB_I_USERNS_VISIBLE;

	return root;
}

static void bsvfs_kill_sb(struct super_block *sb)
{
	void *ns = (void *)kernfs_super_ns(sb);

	kernfs_kill_sb(sb);
	kobj_ns_drop(KOBJ_NS_TYPE_NET, ns);
}

static struct file_system_type bsvfs_fs_type = {
	.name		= "bsvfs",
	.mount		= bsvfs_mount,
	.kill_sb	= bsvfs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};

int __init bsvfs_init(void)
{
	int err;

	bsvfs_root = kernfs_create_root(NULL, 
				KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK, NULL);

	if (IS_ERR(bsvfs_root))
		return PTR_ERR(bsvfs_root);

	bsvfs_root_kn = bsvfs_root->kn;

	err = register_filesystem(&bsvfs_fs_type);
	if (err) {
		kernfs_destroy_root(bsvfs_root);
		return err;
	}

	return 0;
}
device_initcall(bsvfs_init);
