
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/* header of bitmap */
#include "bits32_trie.h"
#include "bits32_trie_debug.h"
#include "bits32_trie_insert.h"
#include "bits32_trie_lookup.h"
#include "bits32_trie_remove.h"
#include "bits32_trie_new.h"

/*  pls keep this default value for demo, you can add new after them  */
b32t_key g_tkey_list[] = {
    0x12345678,
    0x87654321,
    0x11111111,
    0x22222222,
};
struct b32t_trie *gp_trie = NULL;
static __init int bits32_trie_init(void)
{
    int index;
    struct b32t_fib_alias *pfa_new;
    struct b32t_key_vector *leaf;
    gp_trie = b32t_trie_new();
    if(gp_trie == NULL){
        printk("null trie\n");
        return -1;
    }
    printk("%s:[%d - %d]", __func__, (unsigned int)sizeof(g_tkey_list), (unsigned int)sizeof(b32t_key));
    printk("~~~~~~~~  start insert  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    for(index = 0; index < sizeof(g_tkey_list)/sizeof(b32t_key); index++){
        pfa_new = kmalloc(sizeof(struct b32t_fib_alias) + sizeof(char), GFP_KERNEL);
        pfa_new->fa_data[0] = index;
        printk("%s insert leaf[0x%x] data[0x%x]\n", __func__, g_tkey_list[index], index);
        b32t_leaf_insert(gp_trie, g_tkey_list[index], pfa_new);
    }
    printk("~~~~~~~~   end insert   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printk("--------------------------------------------------------------\n");
    printk("~~~~~~~~   now let's look the trie view ~~~~~~~~~~~~~~~~~\n");
    b32t_print_trie(gp_trie);
    printk("~~~~~~~~   trie view end, it's as what you think??  ~~~~~\n");
    printk("--------------------------------------------------------------\n");

    printk("~~~~~~~~   test lookup  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printk("%s lookup[0x%x]index[0x%x]\n", __func__, g_tkey_list[index/2], index/2);
    leaf = b32t_find_leaf(gp_trie, g_tkey_list[index/2], false);
    if(leaf){
        printk("%s it seems ok, found leaf->key[0x%x]\n", __func__, leaf->key);
    }else{
        printk("%s null leaf\n", __func__);
    }
    
    printk("~~~~~~~~  test remove  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printk("%s remove[0x%x]index[0x%x]\n", __func__, g_tkey_list[2], 2);
	b32t_leaf_delete(gp_trie, g_tkey_list[2]);
	leaf = b32t_find_leaf(gp_trie, g_tkey_list[2], false);
    if(leaf){
        printk("%s it seems wrong, found leaf->key[0x%x] after del\n", __func__, leaf->key);
    }else{
        printk("%s it seems ok, found no leaf->key[0x%x] after del\n", __func__, g_tkey_list[2]);
    }
    printk("%s remove[%x]index[%x]\n", __func__, g_tkey_list[0], 0);
    b32t_leaf_delete(gp_trie, g_tkey_list[0]);
    leaf = b32t_find_leaf(gp_trie, g_tkey_list[0], false);
    if(leaf){
        printk("%s it seems wrong, found leaf->key[0x%x] after del\n", __func__, leaf->key);
    }else{
        printk("%s it seems ok, found no leaf->key[0x%x] after del\n", __func__, g_tkey_list[0]);
    }
    printk("~~~~~~~~  now let's check the trie if it same as what you think  ~~~~\n");
    b32t_print_trie(gp_trie);
    printk("%s finish\n", __func__);
	return 0;
}
static __exit void bits32_trie_exit(void)
{
	printk("%s init\n", __func__);
}
module_init(bits32_trie_init);
module_exit(bits32_trie_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("MISC Device Driver");

