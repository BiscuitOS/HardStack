
#ifndef __BITS32_TRIE_H__
#define __BITS32_TRIE_H__

#include <linux/list.h>
#include <linux/slab.h>

typedef unsigned int b32t_key;
#define B32T_KEYLENGTH	(8*sizeof(b32t_key))

struct b32t_key_vector {
	b32t_key key;
	unsigned char pos;
	unsigned char bits;
	union {
		/* This list pointer if valid if (pos | bits) == 0 (LEAF) */
		struct hlist_head leaf;
		/* This array is valid if (pos | bits) > 0 (TNODE) */
		struct b32t_key_vector *tnode[0];
	};
};

struct b32t_trie {
	struct b32t_key_vector kv[1];
};

struct b32t_tnode {
	struct b32t_key_vector *parent;
	struct b32t_key_vector kv[1];
};

struct b32t_fib_alias {
	struct hlist_node	fa_list;
	unsigned char fa_data[0];
};


#define B32T_IS_TNODE(n)	((n)->bits)
#define B32T_IS_LEAF(n)	(!(n)->bits)
#define B32T_IS_TRIE(n)	((n)->pos >= B32T_KEYLENGTH)

#define b32t_get_child(tn, i) (tn)->tnode[i]
//#define b32t_get_cindex(key, kv) (((key) ^ (kv)->key) >> (kv)->pos)
static inline unsigned long b32t_get_cindex(b32t_key key, struct b32t_key_vector *kv){
  unsigned long cindex = 0;
  bitmap_xor(&cindex, (const unsigned long*)&key, (const unsigned long*)&kv->key, B32T_KEYLENGTH);
  cindex &= 0x00000000ffffffff;
  cindex >>= kv->pos;
  return (b32t_key)cindex;
}
#define B32T_TNODE_SIZE(n)	offsetof(struct b32t_tnode, kv[0].tnode[n])
#define B32T_LEAF_SIZE	B32T_TNODE_SIZE(1)

#define B32T_NODE_INIT_PARENT(n, p) (b32t_tn_info(n)->parent = p)
#define b32t_node_free(n) kfree(b32t_tn_info(n))

static inline struct b32t_tnode *b32t_tn_info(struct b32t_key_vector *kv){
	return container_of(kv, struct b32t_tnode, kv[0]);
}

static inline unsigned long b32t_get_index(b32t_key key, struct b32t_key_vector *kv){
	unsigned long index = key ^ kv->key;

	if ((BITS_PER_LONG <= B32T_KEYLENGTH) && (B32T_KEYLENGTH == kv->pos))
		return 0;

	return index >> kv->pos;
}
static inline unsigned long b32t_child_length(const struct b32t_key_vector *tn){
	return (1ul << tn->bits) & ~(1ul);
}


/* Add a child at position i overwriting the old value.
 * Update the value of full_children and empty_children.
 */
static inline void b32t_put_child_index(struct b32t_key_vector *tn, unsigned long i,
		      struct b32t_key_vector *n){
    if(B32T_IS_TRIE(tn)){
        if(i != 0){
            printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__);
            panic("BUG!");
        }else{
            tn->tnode[i] = n;
        }
        return;
    }
	if(i >= b32t_child_length(tn)){
		printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__);
		panic("BUG!");
	}
	tn->tnode[i] = n;
}

static inline void b32t_put_child_key(struct b32t_key_vector *tp, b32t_key key,
				  struct b32t_key_vector *n){
	if (B32T_IS_TRIE(tp))
		tp->tnode[0] = n;
	else
		b32t_put_child_index(tp, b32t_get_index(key, tp), n);
}

static inline void b32t_node_set_parent(struct b32t_key_vector *n, struct b32t_key_vector *tp){
	if (n)
		b32t_tn_info(n)->parent = tp;
}

#endif/*  __BITS32_TRIE_H__  */
