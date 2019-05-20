#ifndef _TREE23_H
#define _TREE23_H
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
struct tree23_node {
	struct tree23_node *left;
	struct tree23_node *middle;
	struct tree23_node *mid_right;
	struct tree23_node *right;
	struct tree23_node *parent;
	int ldata;
	int mdata;
	int rdata;
	bool is2node;
	bool is3node;
	bool is4node;
};

struct tree23_root {
	struct tree23_node *root;
};

#define TREE23_ROOT (struct tree23_root) { NULL, }

/* Simply creates and initializes a 2-3 tree. */
struct tree23_root *tree23_root_init();

/* Deletes and clears all data set by the tree. */
void tree23_deltree(struct tree23_root * root);

/* Inserts a value into the tree. */
void tree23_insert(float val, struct tree23_root * root);

/* Removes a value from the tree. */
void tree23_erase(float val, struct tree23_root * root);

/* Prints all values of the tree out, in order, using a depth-first 
 * traversal. */
void tree23_print(struct tree23_node * root);

bool isvalid(struct tree23_node * curr);
#endif
