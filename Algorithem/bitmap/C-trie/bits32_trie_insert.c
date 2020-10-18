
#include "bits32_trie_insert.h"
#include "bits32_trie_new.h"
#include "bits32_trie_lookup.h"

static int b32t_insert_node(struct b32t_trie *t, struct b32t_key_vector *tp,
			   struct b32t_fib_alias *new, b32t_key key){
	struct b32t_key_vector *n, *l;
	l = b32t_leaf_new(key, new);
	if (!l)
		goto noleaf;

	n = b32t_get_child(tp, b32t_get_index(key, tp));
	if (n) {
		struct b32t_key_vector *tn;
        /*  tips: can use bitmap_xor instead of key ^ n->key, reference to b32t_get_cindex  */
		tn = b32t_tnode_new(key, __fls(key ^ n->key), 1);
		if (!tn)
			goto notnode;
		/* initialize routes out of node */
		B32T_NODE_INIT_PARENT(tn, tp);
		b32t_put_child_index(tn, b32t_get_index(key, tn) ^ 1, n);

		/* start adding routes into the node */
		b32t_put_child_key(tp, key, tn);
		b32t_node_set_parent(n, tn);

		/* parent now has a NULL spot where the leaf can go */
		tp = tn;
	}

	B32T_NODE_INIT_PARENT(l, tp);
	b32t_put_child_key(tp, key, l);
	return 0;
notnode:
	b32t_node_free(l);
noleaf:
	return -ENOMEM;
}

/*
**  1. check node
**  2. insert into
*/
int b32t_leaf_insert(struct b32t_trie *ptrie, b32t_key key, struct b32t_fib_alias *new){
    struct b32t_key_vector *l, *tp;
    l = b32t_find_node(ptrie, &tp, key);
    if(l){
        printk("key has exist\n");
    }
    return b32t_insert_node(ptrie, tp, new, key);	
}

