#ifndef _RB_TREE_H
#define _RB_TREE_H

/* Stand defined without include file */

#undef NULL
#define NULL ((void *)0)

enum {
	false	= 0,
	true	= 1
};
typedef _Bool	bool;

/* RBtree node */
struct rb_node {
	unsigned long __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__ ((aligned(sizeof(long))));

struct rb_root {
	struct rb_node *rb_node;
};

#define rb_parent(r)	((struct rb_node *)((r)->__rb_parent_color & ~3))

#define RB_ROOT (struct rb_root) { NULL, }

#define RB_RED		0
#define RB_BLACK	1

#define __rb_color(pc)		((pc) & 1)
#define __rb_is_black(pc)	__rb_color(pc)
#define __rb_is_red(pc)		(!__rb_color(pc))
#define rb_color(rb)		__rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)		__rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)		__rb_is_black((rb)->__rb_parent_color)

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
                                struct rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

static inline void rb_set_parent_color(struct rb_node *rb,
				       struct rb_node *p, int color)
{
	rb->__rb_parent_color = (unsigned long)p | color;
}

static inline void
__rb_change_child(struct rb_node *old, struct rb_node *new,
		  struct rb_node *parent, struct rb_root *root)
{
	if (parent) {
		if (parent->rb_left == old)
			parent->rb_left = new;
		else
			parent->rb_right = new;
	} else
		root->rb_node = new;
}

extern void rb_insert_color(struct rb_node *, struct rb_root *);

#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)
/**
 * container_of - cast a member of a structure out to the containing structure.
 * @ptr:          the pointer to the member.
 * @type:         the type of the container struct this is embedded in.
 * @member:       the name of the member within the struct.
 */
#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	((type *)(__mptr - offsetof(type, member))); })

#endif
