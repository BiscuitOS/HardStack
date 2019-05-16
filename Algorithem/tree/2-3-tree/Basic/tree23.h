#ifndef _TREE23_H
#define _TREE23_H

#undef NULL
#define NULL ((void *)0)

/* 2-3-tree node 
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
	unsigned long ldata;
	unsigned long right;
	unsigned long rdata;
	bool is2node;
	bool is3node;
	bool is4node;
};

/* 2-3-tree root */
struct tree23_root {
	struct tree23_node *node;
};

#define TREE23_ROOT_INIT (struct tree23_root) { NULL, }

#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)
/**
 * container_of - cast a member of a structure out to the containing structure.
 * @ptr:          the pointer to the member.
 * @type:         the type of the container struct this is embedded in.
 * @member:       the name of the member within the struct.
 */
#define container_of(ptr, type, member) ({                              \
        void *__mptr = (void *)(ptr);                                   \
        ((type *)(__mptr - offsetof(type, member))); })



#endif
