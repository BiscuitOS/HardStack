/*
 * bsfs dir
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/slab.h>

#include "linux/bsfs.h"

static DEFINE_SPINLOCK(bsfs_idr_lock);
DEFINE_MUTEX(bsfs_mutex);

#define rb_to_kn(X)	rb_entry((X), struct bsfs_node, rb)

static struct bsfs_node *__bsfs_new_node(struct bsfs_root *root,
				const char *name, umode_t mode,
				kuid_t uid, kgid_t gid,
				unsigned flags)
{
	struct bsfs_node *kn;
	u32 gen;
	int cursor;
	int ret;

	name = kstrdup_const(name, GFP_KERNEL);
	if (!name)
		return NULL;

	kn = kmem_cache_zalloc(bsfs_node_cache, GFP_KERNEL);
	if (!kn)
		goto err_out1;

	idr_preload(GFP_KERNEL);
	spin_lock(&bsfs_idr_lock);
	cursor = idr_get_cursor(&root->ino_idr);
	ret = idr_alloc_cyclic(&root->ino_idr, kn, 1, 0, GFP_ATOMIC);
	if (ret >= 0 && ret < cursor)
		root->next_generation++;
	gen = root->next_generation;
	spin_unlock(&bsfs_idr_lock);
	idr_preload_end();
	if (ret < 0)
		goto err_out2;
	kn->id.ino = ret;
	kn->id.generation = gen;

	smp_mb__before_atomic();
	atomic_set(&kn->count, 1);
	atomic_set(&kn->active, BS_DEACTIVATED_BIAS);
	RB_CLEAR_NODE(&kn->rb);

	kn->name = name;
	kn->mode = mode;
	kn->flags = flags;

	if (!uid_eq(uid, GLOBAL_ROOT_UID) || !gid_eq(gid, GLOBAL_ROOT_GID)) {
		struct iattr iattr = {
			.ia_valid = ATTR_UID | ATTR_GID,
			.ia_uid = uid,
			.ia_gid = gid,
		};
		ret = __bsfs_setattr(kn, &iattr);
		if (ret < 0)
			goto err_out3;
	}

	return kn;

err_out3:
	idr_remove(&root->ino_idr, kn->id.ino);
err_out2:
	kmem_cache_free(bsfs_node_cache, kn);
err_out1:
	kfree_const(name);
	return NULL;
}

static struct bsfs_node *bsfs_leftmost_descendant(struct bsfs_node *pos)
{
	struct bsfs_node *last;

	while (true) {
		struct rb_node *rbn;

		last = pos;

		if (bsfs_type(pos) != BSFS_DIR)
			break;

		rbn = rb_first(&pos->dir.children);
		if (!rbn)
			break;

		pos = rb_to_kn(rbn);
	}
	return last;
}

static struct bsfs_node *bsfs_next_descendant_post(struct bsfs_node *pos,
				struct bsfs_node *root)
{
	struct rb_node *rbn;

	lockdep_assert_held(&bsfs_mutex);

	if (!pos)
		return bsfs_leftmost_descendant(root);

	if (pos == root)
		return NULL;

	rbn = rb_next(&pos->rb);
	if (rbn)
		return bsfs_leftmost_descendant(rb_to_kn(rbn));

	return pos->parent;
}

void bsfs_activate(struct bsfs_node *kn)
{
	struct bsfs_node *pos;

	mutex_lock(&bsfs_mutex);

	pos = NULL;
	while ((pos = bsfs_next_descendant_post(pos, kn))) {
		if (!pos || (pos->flags & BSFS_ACTIVATED))
			continue;

		WARN_ON_ONCE(pos->parent && RB_EMPTY_NODE(&pos->rb));
		WARN_ON_ONCE(atomic_read(&pos->active) != BS_DEACTIVATED_BIAS);

		atomic_sub(BS_DEACTIVATED_BIAS, &pos->active);
		pos->flags |= BSFS_ACTIVATED;
	}
	mutex_unlock(&bsfs_mutex);
}

struct bsfs_root *bsfs_create_root(struct bsfs_syscall_ops *scops,
			unsigned int flags, void *priv)
{
	struct bsfs_root *root;
	struct bsfs_node *kn;

	printk("WWWW\n");
	root = kzalloc(sizeof(*root), GFP_KERNEL);
	if (!root)
		return ERR_PTR(-ENOMEM);

	idr_init(&root->ino_idr);
	INIT_LIST_HEAD(&root->supers);
	root->next_generation = 1;

	kn = __bsfs_new_node(root, "", S_IFDIR | S_IRUGO | S_IXUGO,
			GLOBAL_ROOT_UID, GLOBAL_ROOT_GID,
			BSFS_DIR);
	if (!kn) {
		idr_destroy(&root->ino_idr);
		kfree(root);
	}

	kn->priv = priv;
	kn->dir.root = root;

	root->syscall_ops = scops;
	root->flags = flags;
	root->kn = kn;
	init_waitqueue_head(&root->deactivate_waitq);

	if (!(root->flags & BSFS_ROOT_CREATE_DEACTIVATED))
		bsfs_activate(kn);

	return root;
}
