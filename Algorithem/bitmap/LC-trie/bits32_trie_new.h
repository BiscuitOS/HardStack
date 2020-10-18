
#ifndef __BITS32_TRIE_NEW_H__
#define __BITS32_TRIE_NEW_H__

#include "bits32_trie.h"

struct b32t_key_vector *b32t_leaf_new(b32t_key key, struct b32t_fib_alias *fa);
struct b32t_key_vector *b32t_tnode_new(b32t_key key, int pos, int bits);
struct b32t_trie *b32t_trie_new(void);

#endif/*  __BITS32_TRIE_NEW_H__  */
