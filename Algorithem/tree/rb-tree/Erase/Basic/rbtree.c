/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  (C) 2002  David Woodhouse <dwmw2@infradead.org>
  (C) 2012  Michel Lespinasse <walken@google.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copy from linux/lib/rbtree.c
*/
#include <rbtree.h>

static inline void rb_set_black(struct rb_node *rb)
{
	rb->__rb_parent_color |= RB_BLACK;
}

static inline struct rb_node *rb_red_parent(struct rb_node *red)
{
	return (struct rb_node *)red->__rb_parent_color;
}

/*
 * Helper function for rotations:
 * - old's parent and color get assigned to new
 * - old get assigned new as a parent and 'color' as color.
 */
static inline void 
__rb_rotate_set_parents(struct rb_node *old, struct rb_node *new,
		       struct rb_root *root, int color)
{
	struct rb_node *parent = rb_parent(old);
	new->__rb_parent_color = old->__rb_parent_color;
	rb_set_parent_color(old, new, color);
	__rb_change_child(old, new, parent, root);
}

static inline void
__rb_insert(struct rb_node *node, struct rb_root *root,
	    bool newleft, struct rb_node **leftmost,
	    void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
	struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

	if (newleft)
		*leftmost = node;

	while (true) {
		/*
		 * Loop invariant: node is red.
		 */
		if (!parent) {
			/*
			 * The inserted node is root. Either this is the
			 * first node, or we recursed at Case 1 below and
			 * are no longer violating 4).
			 */
			rb_set_parent_color(node, NULL, RB_BLACK);
			break;
		}

		/*
		 * If there is a block parent, we are done.
		 * Otherwise, take some corrective action as,
		 * per 4), we don't want a red root or two
		 * consecutive red node.
		 */
		if (rb_is_black(parent))
			break;

		gparent = rb_red_parent(parent);

		tmp = gparent->rb_right;
		if (parent != tmp) { /* parent == gparent->rb_left */
			if (tmp && rb_is_red(tmp)) {
				/*
				 * Case 1 - node's uncle is red (color flips)
				 *
				 *       G            g
				 *      / \          / \
				 *     p   u  -->   P   U
				 *    /            /
				 *   n            n
				 *
				 * However, since g's parent might be red, and
				 * 4) does not allow this, we need to recurse
				 * at g.
				 */
				rb_set_parent_color(tmp, gparent, RB_BLACK);
				rb_set_parent_color(parent, gparent, RB_BLACK);
				node = gparent;
				parent = rb_parent(node);
				rb_set_parent_color(node, parent, RB_RED);
				continue;
			}

			tmp = parent->rb_right;
			if (node == tmp) {
				/*
				 * Case 2 - node's uncle is black and node is
				 * the parent's right child (left rotate at
				 * parent).
				 *
				 *      G             G
				 *     / \           / \
				 *    p   U  -->    n   U
				 *     \           /
				 *      n         p
				 *
				 * This still leaves us in volation of 4), the
				 * continuation into Case 3 will fix that.
				 */
				tmp = node->rb_left;
				parent->rb_right = tmp;
				node->rb_left = parent;
				if (tmp)
					rb_set_parent_color(tmp, parent, 
							    RB_BLACK);
				rb_set_parent_color(parent, node, RB_RED);
				augment_rotate(parent, node);
				parent = node;
				tmp = node->rb_right;
			}

			/*
			 * Case 3 - node's uncle is black and node is
			 * the parent's left child (right rotate at gparent)
			 *
			 *        G           P
                         *       / \         / \
                         *      p   U  -->  n   g
                         *     /                 \
                         *    n                   U
			 */
			gparent->rb_left = tmp;
			parent->rb_right = gparent;
			if (tmp)
				rb_set_parent_color(tmp, gparent, RB_BLACK);
			__rb_rotate_set_parents(gparent, parent, root, RB_RED);
			augment_rotate(gparent, parent);
			break;
		} else { /* parent == gparent->rb_right */
			tmp = gparent->rb_left;
			if (tmp && rb_is_red(tmp)) {
				/* Case 1 - color flips 
				 *
                                 *       G            g
                                 *      / \          / \
                                 *     U   P  -->   U   P
                                 *          \            \
                                 *           n            n
                                 *
				 * However, since g's parent might be reg, and
				 * 4) does not allow this, we need to recurse
				 * at g.
				 */
				rb_set_parent_color(tmp, gparent, RB_BLACK);
				rb_set_parent_color(parent, gparent, RB_BLACK);
				node = gparent;
				parent = rb_parent(node);
				rb_set_parent_color(node, parent, RB_RED);
				continue;
			}

			tmp = parent->rb_left;
			if (node == tmp) {
				/* Case 2 - right rotate at parent
				 *
                                 *      G             G
                                 *     / \           / \
                                 *    U   p  -->    U   n
                                 *       /               \
                                 *      n                 p
                                 *
				 * This still leaves us in violation of 4), the
				 * continuation into Case 3 will fix that.
				 */
				tmp = node->rb_right;
				parent->rb_left = tmp;
				node->rb_right = parent;
				if (tmp)
					rb_set_parent_color(tmp, parent,
								RB_BLACK);
				rb_set_parent_color(parent, node, RB_RED);
				augment_rotate(parent, node);
				parent = node;
				tmp = node->rb_left;
			}

			/*
			 * Case 3 - left rotate at gparent
			 * 
                         *        G           P
                         *       / \         / \
                         *      U   P  -->  g   n
                         *           \     /
                         *            n   U
			 */
			gparent->rb_right = tmp;
			parent->rb_left = gparent;
			if (tmp)
				rb_set_parent_color(tmp, gparent, RB_BLACK);
			__rb_rotate_set_parents(gparent, parent, root, RB_RED);
			augment_rotate(gparent, parent);
		}
	}
}

/*
 * Inline version for rb_erase() use - we want to be able to inline
 * and eliminate the dummy_rotate callback there.
 */
static inline void
____rb_erase_color(struct rb_node *parent, struct rb_root *root,
	void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
	struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

	while (true) {
		/*
		 * Loop invariants:
		 * - node is black (or NULL on first iteration)
		 * - node is not the root (parent is not NULL)
		 * - All leaf paths going through parent and node have a
		 *   black node count that is 1 lower than other leaf path.
		 */
		sibling = parent->rb_right;
		if (node != sibling) { /* node == parent->rb_left */
			if (rb_is_red(sibling)) {
				/*
				 * Case 1 - left rotate at parent
				 *
                                 *     P               S
                                 *    / \             / \
                                 *   N   s    -->    p   Sr
                                 *      / \         / \
                                 *     Sl  Sr      N   Sl
				 */
				tmp1 = sibling->rb_left;
				parent->rb_right = tmp1;
				sibling->rb_left = parent;
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				__rb_rotate_set_parents(parent, sibling, root,
							RB_RED);
				augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_right;
			if (!tmp1 || rb_is_black(tmp1)) {
				tmp2 = sibling->rb_left;
				if (!tmp2 || rb_is_black(tmp2)) {
					/*
					 * Case 2 - sibling color flip
					 * (p could be either color here)
					 *
					 *    (p)           (p)
					 *    / \           / \
					 *   N   S    -->  N   s
					 *      / \           / \
					 *     Sl  Sr        Sl  Sr
					 *
					 * This leaves us violating 5) which
					 * can be fixed by flipping p to back
					 * if it was red, or by recursing at p
					 * p is red when coming from Case 1.
					 */
					rb_set_parent_color(sibling, parent,
								RB_RED);
					if (rb_is_red(parent))
						rb_set_black(parent);
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/*
				 * Case 3 - right rotate at sibling
				 * (p could be either color here)
				 *
				 *   (p)           (p)
				 *   / \           / \
				 *  N   S    -->  N   sl
				 *     / \             \
				 *    sl  Sr            S
				 *                       \
				 *                        Sr
				 *
				 * Note: p might be red, and then both
				 * p and sl are red after rotation(which
				 * breaks property 4). this is fixed in
				 * Case 4 (in __rb_rotate_set_parents()
				 *         which set sl the color of p
				 *         and set p RB_BLACK)
				 *
				 *   (p)            (sl)
				 *   / \            /  \
				 *  N   sl   -->   P    S
				 *       \        /      \
				 *        S      N        Sr
				 *         \
				 *          Sr
				 */
				tmp1 = tmp2->rb_right;
				sibling->rb_left = tmp1;
				tmp2->rb_right = sibling;
				parent->rb_right = tmp2;
				if (tmp1)
					rb_set_parent_color(tmp1, sibling,
								RB_BLACK);
				augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/*
			 * Case 4 - left rotate at parent + color flips
			 * (p and sl count be either color here.
			 * After rotation, p becomes black, s acquires
			 * p's color, and sl keeps its color)
			 *
	 		 *      (p)             (s)
			 *      / \             / \
			 *     N   S     -->   P   Sr
			 *        / \         / \
			 *      (sl) sr      N  (sl)
			 */
			tmp2 = sibling->rb_left;
			parent->rb_right = tmp2;
			sibling->rb_left = parent;
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2)
				rb_set_parent(tmp2, parent);
			__rb_rotate_set_parents(parent, sibling, root,
						RB_BLACK);
			augment_rotate(parent, sibling);
			break;
		} else {
			sibling = parent->rb_left;
			if (rb_is_red(sibling)) {
				/* Case 1- right rotate at parent */
				tmp1 = sibling->rb_right;
				parent->rb_left = tmp1;
				sibling->rb_right = parent;
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				__rb_rotate_set_parents(parent, sibling, root,
							RB_RED);
				augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_left;
			if (!tmp1 || rb_is_black(tmp1)) {
				tmp2 = sibling->rb_right;
				if (!tmp2 || rb_is_black(tmp2)) {
					/* Case 2 - sibling color flip */
					rb_set_parent_color(sibling, parent,
							RB_RED);
					if (rb_is_red(parent))
						rb_set_black(parent);
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent)
							continue;
					}
					break;
				}
				/* Case 3 - left rotate at sibling */
				tmp1 = tmp2->rb_left;
				sibling->rb_right, tmp1;
				tmp2->rb_left = sibling;
				parent->rb_left = tmp2;
				if (tmp1)
					rb_set_parent_color(tmp1, sibling,
							    RB_BLACK);
				augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/* Case 4 - right rotate at parent + color flips */
			tmp2 = sibling->rb_right;
			parent->rb_left = tmp2;
			sibling->rb_right = parent;
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2)
				rb_set_parent(tmp2, parent);
			__rb_rotate_set_parents(parent, sibling, root,
						RB_BLACK);
			augment_rotate(parent, sibling);
			break;
		}
	}
}

/*
 * Non-augmented rbtree manipulation functions.
 *
 * We use dummy augmented callbacks here, and have the compiler optimize them
 * out of the rb_insert_color() and rb_erase() function definitions.
 */

static inline void dummy_propagate(struct rb_node *node, struct rb_node *stop) {}

static inline void dummy_copy(struct rb_node *old, struct rb_node *new) {}
static inline void dummy_rotate(struct rb_node *old, struct rb_node *new) {}

static const struct rb_augment_callbacks dummy_callbacks = {
	.propagate = dummy_propagate,
	.copy = dummy_copy,
	.rotate = dummy_rotate
};

void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	__rb_insert(node, root, false, NULL, dummy_rotate);
}

void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *rebalance;
	rebalance = __rb_erase_augmented(node, root,
					 NULL, &dummy_callbacks);
	if (rebalance)
		____rb_erase_color(rebalance, root, dummy_rotate);
}

/*
 * This function returns the first node (in sort order) of the tree.
 */
struct rb_node *rb_first(const struct rb_root *root)
{
	struct rb_node *n;

	n = root->rb_node;
	if (!n)
		return NULL;
	while (n->rb_left)
		n = n->rb_left;
	return n;
}

struct rb_node *rb_last(const struct rb_root *root)
{
	struct rb_node *n;

	n = root->rb_node;
	if (!n)
		return NULL;
	while (n->rb_right)
		n = n->rb_right;
	return n;
}

struct rb_node *rb_next(const struct rb_node *node)
{
	struct rb_node *parent;

	if (RB_EMPTY_NODE(node))
		return NULL;

	/*
	 * If we have a right-hand child, go down and then left as far
	 * as we can.
	 */
	if (node->rb_right) {
		node = node->rb_right;
		while (node->rb_left)
			node = node->rb_left;
		return (struct rb_node *)node;
	}
	
	/*
	 * No right-hand children. Everything down and left is smaller than us,
	 * so any 'next' node must be in the general direction of our parent.
	 * Go up the tree; any time the ancestor is a right-hand child of its
	 * parent, keep going up. First time it's a left-hand child of its
	 * parent, said parent is our 'next' node.
	 */
	while ((parent = rb_parent(node)) && node == parent->rb_right)
		node = parent;
	return parent;
}
