/*
 * "tree23.h", by Sean Soderman
 * Specification of 2-3 tree functions.
 */
#ifndef STDBOOL_H
#include <stdbool.h>
#endif

#ifndef STDINT_H
#include <stdint.h>
#endif



/*
 * Defines a node ptr. mid_right and mdata are both 
 * temporary, used in the case when a 3-node overflows. Parent is
 * used to give child nodes access to their ancestors.
 */
typedef struct n {
   struct n * left;
   struct n * middle;
   struct n * mid_right;
   struct n * right;
   struct n * parent;
   float ldata;
   float mdata;
   float rdata;
   bool is2node;
   bool is3node;
   bool is4node;
}node;

typedef struct t {
   node * root;
   //uint64_t size;
}tree;

//Simply creates and initializes a 2-3 tree.
tree * create();

//Deletes and clears all data set by the tree.
void deltree(tree * root);

//Inserts a value into the tree.
void insert(float val, tree * root);

//Removes a value from the tree.
void rmval(float val, tree * root);

//Prints all values of the tree out, in order, using a depth-first traversal.
void treeprint(node * root);

bool isvalid(node * curr);
