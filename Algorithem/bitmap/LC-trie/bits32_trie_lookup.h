
#ifndef __BITS32_TRIE_LOOKUP_H__
#define __BITS32_TRIE_LOOKUP_H__

#include "bits32_trie.h"

struct b32t_key_vector *b32t_find_node(struct b32t_trie *t, struct b32t_key_vector **tp, u32 key);
struct b32t_key_vector *b32t_find_leaf(struct b32t_trie *t, b32t_key key, unsigned char remove);


#endif/*  __BITS32_TRIE_LOOKUP_H__  */
