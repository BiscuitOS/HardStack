#ifndef _ANON_INTERVAL_TREE_H
#define _ANON_INTERVAL_TREE_H

void BiscuitOS_anon_vma_interval_tree_insert(struct anon_vma_chain *node,
                                             struct rb_root_cached *root);
void BiscuitOS_anon_vma_interval_tree_remove(struct anon_vma_chain *node,
                                   struct rb_root_cached *root);

struct anon_vma_chain *
BiscuitOS_anon_vma_interval_tree_iter_first(struct rb_root_cached *root,
                                  unsigned long first, unsigned long last);

struct anon_vma_chain *
BiscuitOS_anon_vma_interval_tree_iter_next(struct anon_vma_chain *node,
                                 unsigned long first, unsigned long last);

#define BiscuitOS_anon_vma_interval_tree_foreach(avc, root, start, last)           \
        for (avc = BiscuitOS_anon_vma_interval_tree_iter_first(root, start, last); \
             avc; \
	     avc = BiscuitOS_anon_vma_interval_tree_iter_next(avc, start, last))

#endif
