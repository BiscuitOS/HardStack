
#include "bits32_trie_remove.h"
#include "bits32_trie_lookup.h"

static void b32t_flush_parent(struct b32t_key_vector *n){
	int index;
	struct b32t_key_vector *tn;
	b32t_key key;
	if(n == NULL){
		return;
	}
	for(index = 0; index < b32t_child_length(n); index++){
		if(b32t_get_child(n, index)){
			return;
		}
	}

	tn = b32t_tn_info(n)->parent;
    if(tn == NULL){
        printk("%s no parent\n", __func__);
        return;
    }
	key = n->key;
	index = b32t_get_cindex(key, tn);
	b32t_node_free(n);
	b32t_put_child_index(tn, index, NULL);
	b32t_flush_parent(tn);
}
int b32t_leaf_delete(struct b32t_trie *ptrie, b32t_key key){
    struct b32t_key_vector *l, *tp;
	struct b32t_fib_alias *fa;
	struct hlist_node *hn;
	l = b32t_find_leaf(ptrie, key, true);
	if (!l)
		return 0;
	tp = b32t_tn_info(l)->parent;
	if(hlist_empty(&l->leaf)){
		b32t_node_free(l);
		b32t_flush_parent(tp);
		return 0;
	}
	hlist_for_each_entry_safe(fa, hn, &l->leaf, fa_list) {
		hlist_del(&fa->fa_list);
		kfree(fa);
    }
	b32t_node_free(l);
	b32t_flush_parent(tp);
	return 0;
}

