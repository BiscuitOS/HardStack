
#include "bits32_trie_lookup.h"

struct b32t_key_vector *b32t_find_node(struct b32t_trie *t, struct b32t_key_vector **tp, u32 key){
	struct b32t_key_vector *pn, *n = t->kv;
	unsigned long index = 0;
	do {
		pn = n;
		n = b32t_get_child(n, index);

		if (!n)
			break;

		index = b32t_get_cindex(key, n);

		if (index >= (1ul << n->bits)) {
			n = NULL;
			break;
		}

		/* keep searching until we find a perfect match leaf or NULL */
	} while (B32T_IS_TNODE(n));

	*tp = pn;

	return n;
}

struct b32t_key_vector *b32t_find_leaf(struct b32t_trie *t, b32t_key key, unsigned char remove){
	struct b32t_key_vector *n, *tp;
	unsigned long index;

	tp = t->kv;
	index = 0;

	n = b32t_get_child(tp, index);
	if (!n) {
		return NULL;
	}
    if(B32T_IS_LEAF(n)){
        if(n->key == key){
            if(remove)
                tp->tnode[0] = NULL;
            return n;
        }
        else
            return NULL;
    }

	for (;;) {
		index = b32t_get_cindex(key, n);
		if (index >= (1ul << n->bits))
			return NULL;

		tp = n;
		n = b32t_get_child(tp, index);
		if (n == NULL)
			return NULL;
		if (B32T_IS_LEAF(n)){
			if(n->key == key){
				if(remove)
					b32t_put_child_index(tp, index, NULL);
                break;
			}else
                return NULL;
		}
	}
    return n;
}

