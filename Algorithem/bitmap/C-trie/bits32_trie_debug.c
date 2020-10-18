
#include "bits32_trie_debug.h"

static int g_offset;

static void b32t_print_leaf(struct b32t_key_vector *l, int cindex){
    struct b32t_tnode *tn;
    int index;
    struct b32t_fib_alias *fa;
    char offset_char[300 * 2] = {0};

    tn = b32t_tn_info(l);
    for(index = 0; index < g_offset; index++){
        if(index >= 256)
            break;
        offset_char[index * 2] = '-';
        offset_char[index * 2 + 1] = ' ';
    }
    offset_char[index * 2] = '\0';
    if(hlist_empty(&l->leaf)){
        printk("%sleaf[0x%x]:pos[0x%x]bits[0x%x]cindex[0x%x]", offset_char, l->key, l->pos, l->bits, cindex);
        return;
    }
    hlist_for_each_entry(fa, &l->leaf, fa_list) {
        printk("%sleaf[0x%x]:pos[0x%x]bits[0x%x]data[0x%x]cindex[0x%x]", 
                  offset_char, l->key, l->pos, l->bits, fa->fa_data[0], cindex);
    }
}
static void b32t_print_each_tnode(struct b32t_key_vector *n, int cindex){
    struct b32t_tnode *tn;
    int index;
    char offset_char[300 * 2] = {0};

    tn = b32t_tn_info(n);
    for(index = 0; index < g_offset; index++){
        if(index >= 256)
            break;
        offset_char[index * 2] = '-';
        offset_char[index * 2 + 1] = ' ';
    }
    printk("%stnode[0x%x]:pos[0x%x]bits[0x%x]cindex[0x%x]", offset_char, n->key, n->pos, n->bits, cindex);
}

void b32t_print_tnode(struct b32t_key_vector *pn, int cindex){
    int index;
    struct b32t_key_vector *n;
    /*  tnode  */
    if(pn == NULL){
        return;
    }
    b32t_print_each_tnode(pn, cindex);
    g_offset++;
    for(index = 0; index < b32t_child_length(pn); index++){
        n = b32t_get_child(pn, index);
        if(n == NULL){
            continue;
        }
        if(B32T_IS_LEAF(n)){
            b32t_print_leaf(n, index);
        }else{
            b32t_print_tnode(n, index);
        }
    }
    g_offset--;
}

void b32t_print_trie(struct b32t_trie *ptrie){
    struct b32t_key_vector *pn;
    struct b32t_key_vector *n;

    if(ptrie == NULL){
        printk("empty trie\n");
        return;
    }
    pn = ptrie->kv;
    n = b32t_get_child(pn, 0);
    g_offset = 0;
	
    if(B32T_IS_LEAF(n)){
        b32t_print_leaf(n, 0);
        return;
    }else{
        b32t_print_tnode(n, 0);
    }
}

