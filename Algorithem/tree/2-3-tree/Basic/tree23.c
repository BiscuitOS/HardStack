/*
 * "tree23.c", by Sean Soderman
 * Implementation of all necessary 2-3 tree functions, as well as
 * auxiliary "helper" functions to cut down on redundant code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <tree23.h>

/*
 * Allows children nodes to determine if they are a left, middle, or
 * right node. Allows for easy error trapping as well.
 */
typedef enum dir {
	left,
	middle,
	right,
	no_parent,
	error
} direction;

/*
 * Denotes the type of operation the modmem function will execute.
 */
typedef enum f {
	GET,
	FREE,
	DEL
} fetch_style;

/* Inserts val into the tree pointed to by n. */
static void minsert(float val, struct tree23_node * n, direction dir);
/* Turns n into a 2-node by inserting val into it. */
static void simpleswap(float val, struct tree23_node * n);
/* Turns n into a 3-node by inserting val into it. */
static void swapsort(float val, struct tree23_node * n);
/* Function that encompasses (almost) all memory management the tree needs. */
static struct tree23_node * modmem(fetch_style f, struct tree23_node * node_to_clear);
/* Helper function for rmval that does all the heavy lifting. */
static struct tree23_node * mrmval(float val, struct tree23_node * top_node);
/* Discerns which child the node is. */
static direction discern_childhood(struct tree23_node * child, struct tree23_node * parent);
/* Validates the 2-3 tree by checking if the ordering of its values are
 * correct. Returns true if the tree passes the test, false otherwise.
 */
bool isvalid(struct tree23_node * curr);

/*
 * Handles the initialization of the tree.
 */
struct tree23_root *tree23_root_init(void) 
{
	struct tree23_root * seed = malloc(sizeof(struct tree23_root));
	seed->root = modmem(GET, NULL);
	return seed;
}

/*
 * Takes care of the deletion of the entire tree, including the tree struct.
 */
void tree23_deltree(struct tree23_root * root) 
{
	(void)modmem(FREE, NULL);
	memset(root, '\0', sizeof(root));
	free(root);
}
/*
 * Takes the value "val" and inserts it into the tree.
 * Grows at the root if necessary.
 */
void tree23_insert(float val, struct tree23_root * root)
{
	struct tree23_node * n = root->root;
	if (n->left || n->right) { //If I am a 2 or 3 node w/ children.
		if (val < n->ldata) {
			minsert(val, n->left, left);
		} else if (n->middle != NULL && val < n->rdata) {
			minsert(val, n->middle, middle);
		} else {
			minsert(val, n->right, right);
		}
	}

	/* 
	 * The root is a temp-4 node w/children, grow a new root and 
	 * right node.
	 */
	if (n->is4node && n->mid_right) {
		/*
		 * Create new root, have old root's parent ptr point to it.
		 * Make sure to clear out the middle data as well.
		 */
		struct tree23_node * new_root = modmem(GET, NULL);
		n->parent = new_root;
		new_root->ldata = n->mdata;
		new_root->is2node = true;
		n->mdata = 0;
		/* Have the new root point to the old one. */
		new_root->left = n;
		/*
		 * Create the new right branch of the tree as well. Migrate 
		 * the proper pointers over (including the parent pointers!)
		 */
		struct tree23_node *new_right = modmem(GET, NULL);
		new_root->right = new_right;
		new_right->parent = new_root;
		new_right->ldata = n->rdata;
		new_right->is2node = true;
		n->rdata = 0;
		n->is4node = false;
		n->is3node = false;
		n->is2node = true;
		new_right->left = n->mid_right;
		new_right->right = n->right;
		n->right = n->middle;
		n->middle = NULL;
		n->mid_right = NULL;
		/* Have grandchild parent pointers point to new right node. */
		new_right->left->parent = new_right;
		new_right->right->parent = new_right;
		root->root = new_root;
	} else if (n->is3node && n->left == NULL) {
		/* 
		 * Initial case of inserting data: a full root node with 
		 * no children. 
		 */
		struct tree23_node *new_root = modmem(GET, NULL);
		new_root->left = n;
		swapsort(val, n);
		struct tree23_node * new_right = modmem(GET, NULL);
		n->parent = new_root;
		new_right->parent = new_root;
		new_root->ldata = n->mdata;
		new_root->is2node = true;
		n->mdata = 0;
		new_root->right = new_right;
		new_right->ldata = n->rdata;
		new_right->is2node = true;
		n->rdata = 0;
		n->is3node = false;
		n->is2node = true;
		root->root = new_root;
	} else if (n->is2node != true && n->left == NULL) {
		n->ldata = val;
		n->is2node = true;
	} else if (n->is2node && n->left == NULL) {
		simpleswap(val, n);
	}
}

/*
 * Prints all values of the tree in order, using depth-first traversal.
 */
void tree23_print(struct tree23_node * root)
{
	if (root->left != NULL)
		tree23_print(root->left);
	printf("ldata: %d\n", root->ldata); 
	if (root->middle != NULL)
		tree23_print(root->middle);
	if (root->is3node)
		printf("rdata: %d\n", root->rdata);
	if (root->right != NULL)
		tree23_print(root->right);
}

/*
 * Helper function for insert. Does all the heavy lifting save for growth
 * at the root node, which is reserved for insert itself.
 */
static void minsert(float val, struct tree23_node * n, direction dir)
{
	/* Shameless copy from insert. The logic is identical... */
	if (n->left || n->right) { //If I am a 2 or 3 node w/ children.
		if (val < n->ldata) {
			minsert(val, n->left, left);
		} else if (n->middle != NULL && val < n->rdata) {
			minsert(val, n->middle, middle);
		} else {
			minsert(val, n->right, right);
		}
	} else if (n->is2node && n->left == NULL) { //I am a leaf 2-node
		simpleswap(val, n);
	} else { //I am a leaf 3-node and I'm ready to overflow!
		swapsort(val, n);
		n->is4node = true;
	}

	/* The node has overflowed! Split accordingly. */
	if (n->is4node) {
		struct tree23_node *parent = n->parent;
		float promoted_val = n->mdata;

		if (parent->is2node) { //Parent is a 2-node
			simpleswap(promoted_val, parent);
			struct tree23_node *new_node = modmem(GET, NULL);
			new_node->parent = parent;
			parent->middle = new_node;
			switch(dir) {
			case left:
				new_node->ldata = n->rdata;
				/*
			 	* Transfer pointers unconditionally, since it 
			 	* wouldn't hurt either way
			 	*/
				new_node->left = n->mid_right;
				new_node->right = n->right;
				if (new_node->left != NULL) {
					new_node->left->parent = new_node;
					new_node->right->parent = new_node;
				}
				n->right = n->middle;
				break;
			case right:
				new_node->ldata = n->ldata;
				n->ldata = n->rdata;
				/* As above, unconditional pointer xfer. */
				new_node->left = n->left;
				new_node->right = n->middle;
				if (new_node->left != NULL) {
					new_node->left->parent = new_node;
					new_node->right->parent = new_node;
				}
				n->left = n->mid_right;
				break;
			}
			n->mid_right = NULL;
			n->middle = NULL;
			n->rdata = 0;
			new_node->is2node = true;
		} else { //Parent is a 3-node.
			swapsort(promoted_val, parent);
			parent->is4node = true;
			struct tree23_node *new_node = modmem(GET, NULL);
			new_node->parent = parent;
			switch(dir) {
			case left: //Rearrange for left
				parent->mid_right = parent->middle;
				parent->middle = new_node;
				new_node->ldata = n->rdata;
				new_node->left = n->mid_right;
				new_node->right = n->right;
				if (new_node->left != NULL) {
					new_node->left->parent = new_node;
					new_node->right->parent = new_node;
				}
				n->right = n->middle;
				break;
			case middle: //Rearrange for middle
				parent->mid_right = new_node;
				new_node->ldata = n->rdata;
				new_node->left = n->mid_right;
				new_node->right = n->right;
				if (new_node->left != NULL) {
					new_node->left->parent = new_node;
					new_node->right->parent = new_node;
				}
				n->right = n->middle;
				break;
			case right: //Rearrange for right
				parent->mid_right = new_node;
				new_node->ldata = n->ldata;
				n->ldata = n->rdata;
				new_node->left = n->left;
				new_node->right = n->middle;
				if (new_node->left != NULL) {
					new_node->left->parent = new_node;
					new_node->right->parent = new_node;
				}
				n->left = n->mid_right;
				break;
			}
			n->rdata = 0;
			n->middle = NULL;
			n->mid_right = NULL;
			new_node->is2node = true;
		}
		n->mdata = 0; //Clean up temp value storage.
		n->is2node = true;
		n->is3node = false;
		n->is4node = false;
	}
}

/*
 * Removes the value "val" from the tree.
 */
void tree23_erase(float val, struct tree23_root * root)
{
	struct tree23_node * top_node = root->root;

	/* If this is just a root with no children.. */
	if (top_node->left == NULL) {
		/* 
	 	 * Works for either 2 or 3-node roots since all nodes 
		 * are initialised to zero.
	 	 */
		if (top_node->is3node) {
			if (top_node->ldata == val) {
				top_node->ldata = top_node->rdata;
				top_node->rdata = 0;
				top_node->is3node = false;
				top_node->is2node = true;
			} else if (top_node->rdata == val) {
				top_node->rdata = 0;
				top_node->is3node = false;
				top_node->is2node = true;
			}
		} else if (top_node->is2node) {
			if (top_node->ldata == val) {
				top_node->ldata = 0;
				top_node->is2node = false;
			}
		}
		return;
	}
   
	struct tree23_node *new_root = mrmval(val, top_node);
	/* If my root node has been cleared... */
	if (new_root != NULL) {
		root->root = new_root;
		modmem(DEL, new_root->parent);
		new_root->parent = NULL;
	}
}

/*
 * Helper function for rmval that does all the heavy lifting.
 */
static struct tree23_node * mrmval(float val, struct tree23_node * top_node)
{
	/* Points to the node with a matching value. */
	struct tree23_node *node_to_swap = NULL;
	struct tree23_node *curr = top_node;

	struct tree23_node *prev = NULL; //Used to restore curr.
	/* Avoids several ifs later when it comes to switching. */
	direction val_to_switch = middle;

	/*
	 * 1st loop: Dive to the bottom, setting up the swap between 
	 * the node with "val" and a leaf node.
	 */
	while (curr != NULL) { //(curr->left != NULL) 
		prev = curr;
		if (curr->ldata == val || (curr->is3node && 
					(curr->rdata == val))) {
			node_to_swap = curr;
			val_to_switch = curr->ldata == val ? left : right;
			/*
	 		 * Once I find the correct value, get to the 
			 * biggest value of the left subtree, or the 
			 * smallest value of the right subtree.
	 		 */
			if (val_to_switch == left)
				curr = curr->left;
			else if (val_to_switch == right)
				curr = curr->right;
		} else if (node_to_swap == NULL) { 
			if (val < curr->ldata) {
				curr = curr->left;
			} else if (curr->is3node && val < curr->rdata) {
				curr = curr->middle;
			} else
				curr = curr->right;
		} else {
			if (val_to_switch == left)
				curr = curr->right;
			else if (val_to_switch == right)
				curr = curr->left;
		}
	}
	curr = prev;
	/*
	 * Switch the greatest value of l. subtree with selected value,
	 * if it was found, then demote the leaf node and clear the duplicate
	 * value.
	 */
	switch(val_to_switch) {
	case left:
		if (curr->is3node) {
			node_to_swap->ldata = curr->rdata;
			curr->rdata = 0;
			curr->is2node = true;
			curr->is3node = false;
		} else {
			node_to_swap->ldata = curr->ldata;
			curr->ldata = 0;
			curr->is2node = false;
		}
		break;
	case right:
		if (curr->is3node) {
			node_to_swap->rdata = curr->ldata;
			curr->ldata = curr->rdata;
			curr->rdata = 0;
			curr->is2node = true;
			curr->is3node = false;
		} else {
			node_to_swap->rdata = curr->ldata;
			curr->ldata = 0;
			curr->is2node = false;
		}
		break;
	/* The value wasn't found! Should NOT happen during diagnostics. */
	case middle:
		return NULL;
	}
	/*
	 * 2nd loop: Pointer reorganisation, traverse upwards when necessary.
	 * Iterate only when my current node is empty.
	 */
	while(!curr->is2node && !curr->is3node) {
		/* Convenience ptrs to reduce no. of following "->". */
		struct tree23_node *parent = curr->parent;
		struct tree23_node *lchild = parent->left;
		struct tree23_node *mchild = parent->middle;
		struct tree23_node *rchild = parent->right;
		/* This is necessary for figuring which branches to 
		 * move, etc.
		 */
		direction which_child = discern_childhood(curr, curr->parent);
		switch(which_child) {
		case error:
			fprintf(stderr, "Error: A child was adopted by "
							"another parent\n");
			return NULL;
		case left:
			if (parent->is3node) {
				/*
				 * Is either sibling a 3-node? If so, move 
				 * vals from parent and sibling over and graft
				 * the sibling's left branch over to curr's
				 * right branch.
				 */
				if (mchild->is3node) {
					curr->ldata = parent->ldata;
					parent->ldata = mchild->ldata;
					mchild->ldata = mchild->rdata;
					mchild->rdata = 0;
					mchild->is3node = false;
					mchild->is2node = true;
					curr->is2node = true;
					curr->right = mchild->left;
					if (curr->right != NULL)
						curr->right->parent = curr;
					mchild->left = mchild->middle;
					mchild->middle = NULL;
				} else if (rchild->is3node) {
					curr->ldata = parent->ldata;
					parent->ldata = mchild->ldata;
					mchild->ldata = parent->rdata;
					parent->rdata = rchild->ldata;
					rchild->ldata = rchild->rdata;
					rchild->rdata = 0;
					rchild->is3node = false;
					rchild->is2node = true;
					curr->is2node = true;
					curr->right = mchild->left;
					if (curr->right != NULL)
						curr->right->parent = curr;
					mchild->left = mchild->right;
					mchild->right = rchild->left;
					if (mchild->right != NULL)
						mchild->right->parent = mchild;
					rchild->left = rchild->middle;
					rchild->middle = NULL;
				} else {
					/* 
					 * Use the parent's "extra" value for
					 * help! This is currently done "my" 
					 * way. If it doesn't work, I'm 
					 * reverting to the default (for all
					 * three cases).
					 */
					curr->ldata = parent->ldata;
					curr->rdata = mchild->ldata;
					parent->ldata = parent->rdata;
					parent->rdata = 0;
					curr->middle = mchild->left;
					curr->right = mchild->right;
					if (curr->middle != NULL && 
							curr->right != NULL) {
						curr->middle->parent = curr;
						curr->right->parent = curr;
					}
					curr->is3node = true;
					parent->is3node = false;
					parent->is2node = true;
					modmem(DEL, mchild);
					parent->middle = NULL;
				} /* End left child 3node case */
			} /* End 3-node parent case */ else { 
				/* Parent is a 2-node. */
				if (rchild->is3node) {
					curr->ldata = parent->ldata;
					parent->ldata = rchild->ldata;
					rchild->ldata = rchild->rdata;
					rchild->rdata = 0;
					rchild->is3node = false;
					rchild->is2node = true;
					curr->is2node = true;
					curr->right = rchild->left;
					rchild->left = rchild->middle;
					rchild->middle = NULL;
					if (curr->right != NULL)
						curr->right->parent = curr;
				} else { 
					/* 
					 * Parent and sibling are 2-nodes.
					 * Merge parent into nonempty sibling 
					 * node.
					 */
					rchild->rdata = rchild->ldata;
					rchild->ldata = parent->ldata;
					parent->ldata = 0;
					rchild->middle = rchild->left;
					rchild->is2node = false;
					rchild->is3node = true;
					rchild->left = curr->left;
					if (rchild->left != NULL)
						rchild->left->parent = rchild;
					modmem(DEL, curr);
					curr = parent;
					curr->is2node = false;
					curr->left = NULL;
					/*
					 * Assign the merged child to the ptr 
					 * that can be safely left "alone"
					 * on subsequent iterations.
					 */
					direction d = discern_childhood(curr,
								curr->parent);
					if (d == left) {
						curr->left = curr->right;
						curr->right = NULL;
					} else if (d == middle) {
						curr->middle = curr->right;
						curr->right = NULL;
					} else if (d == no_parent) {
						return rchild; //curr;
					}
				}
			}
			break; /* End case for empty curr lchild. */
		case right:
			if (parent->is3node) {
				if (mchild->is3node) {
					curr->ldata = parent->rdata;
					parent->rdata = mchild->rdata;
					mchild->rdata = 0;
					mchild->is3node = false;
					mchild->is2node = true;
					curr->is2node = true;
					curr->left = mchild->right;
					if (curr->left != NULL)
						curr->left->parent = curr;
					mchild->right = mchild->middle;
					mchild->middle = NULL;
				} else if (lchild->is3node) {
					curr->ldata = parent->rdata;
					parent->rdata = mchild->ldata;
					mchild->ldata = parent->ldata;
					parent->ldata = lchild->rdata;
					lchild->rdata = 0;
					lchild->is3node = false;
					lchild->is2node = true;
					curr->is2node = true;
					curr->left = mchild->right;
					if (curr->left != NULL)
						curr->left->parent = curr;
					mchild->right = mchild->left;
					mchild->left = lchild->right;
					if (mchild->left != NULL)
						mchild->left->parent = mchild;
					lchild->right = lchild->middle;
					lchild->middle = NULL;
				} else { 
					/*
					 * Make 2 node by bringing parent's
					 * rval down & merging the middle node
					 * in.
					 */
					curr->rdata = parent->rdata;
					curr->ldata = mchild->ldata;
					parent->rdata = 0;
					parent->is3node = false;
					parent->is2node = true;
					curr->is3node = true;
					curr->middle = mchild->right;
					if (curr->middle != NULL)
						curr->middle->parent = curr;
					curr->left = mchild->left;
					if (curr->left != NULL)
						curr->left->parent = curr;
					modmem(DEL, mchild);
					parent->middle = NULL;
				}
			} else { /* Parent is 2node. */
				if (lchild->is3node) {
					curr->ldata = parent->ldata;
					parent->ldata = lchild->rdata;
					lchild->rdata = 0;
					curr->is2node = true;
					lchild->is3node = false;
					lchild->is2node = true;
					curr->left = lchild->right;
					if (curr->left != NULL)
						curr->left->parent = curr;
					lchild->right = lchild->middle;
					lchild->middle = NULL;
				} else {
					/*
					 * Merge parent into sibling node, 
					 * promote curr.
					 */
					lchild->rdata = parent->ldata;
					parent->ldata = 0;
					lchild->middle = lchild->right;
					lchild->right = curr->right;
					lchild->is2node = false;
					lchild->is3node = true;
					parent->is2node = false;
					if (lchild->right != NULL)
						lchild->right->parent = lchild;
					modmem(DEL, curr);
					curr = parent;
					curr->right = NULL;
					direction d = discern_childhood(curr,
							curr->parent);
					if (d == right) {
						curr->right = curr->left;
						curr->left = NULL;
					} else if (d == middle) {
						curr->middle = curr->left;
						curr->left = NULL;
					} else if (d == no_parent) {
						return lchild;
					}
				}
			}
			break;
		case middle:
			/*
			 * Since this is the middle case, it's one hop 
			 * either way, *and* my parent is a guaranteed 3-node.
			 */
			if (lchild->is3node) {
				curr->ldata = parent->ldata;
				parent->ldata = lchild->rdata;
				lchild->rdata = 0;
				lchild->is3node = false;
				lchild->is2node = true;
				curr->is2node = true;
				curr->right = curr->middle;
				curr->middle = NULL;
				curr->left = lchild->right;
				if (curr->left != NULL)
					curr->left->parent = curr;
				lchild->right = lchild->middle;
				lchild->middle = NULL;
			} else if (rchild->is3node) {
				curr->ldata = parent->rdata;
				parent->rdata = rchild->ldata;
				rchild->ldata = rchild->rdata;
				rchild->rdata = 0;
				rchild->is3node = false;
				rchild->is2node = true;
				curr->is2node = true;
				curr->left = curr->middle;
				curr->middle = NULL;
				curr->right = rchild->left;
				if (curr->right != NULL)
					curr->right->parent = curr;
				rchild->left = rchild->middle;
				rchild->middle = NULL;
			} else {
				/*
				 * Use either sibling with parent to make 
				 * new 3-node. Here I'll just use the left
				 * child.
				 */
				lchild->rdata = parent->ldata;
				parent->ldata = parent->rdata;
				parent->rdata = 0;
				parent->is3node = false;
				parent->is2node = true;
				lchild->is2node = false;
				lchild->is3node = true;
				lchild->middle = lchild->right;
				lchild->right = curr->middle;
				if (lchild->right != NULL)
					lchild->right->parent = lchild;
				modmem(DEL, curr);
				curr = lchild;
				parent->middle = NULL;
			}
			break;
		}
	} 
}

/* Discerns which child the node is.
 * Returns: The named branch of the parent the child node is attached to.
 */
direction discern_childhood(struct tree23_node * child, struct tree23_node * parent)
{
	if (parent == NULL)
		return no_parent;
	else if (child == parent->left)
		return left;
	else if (child == parent->right)
		return right;
	else if (child == parent->middle)
		return middle;
	else {
		fprintf(stderr, "Error: Child has a different parent.\n");
		return error;
	}
}

/*
 * Swaps 'val' into the 2-node in such a way that 
 * the left value is smaller than (or equal to) the right one.
 */
static void simpleswap(float val, struct tree23_node * n)
{
	if (val > n->ldata)
		n->rdata = val;
	else {
		n->rdata = n->ldata;
		n->ldata = val;
	}
	n->is2node = false;
	n->is3node = true;
}
/*
 * Swaps 'val' into the 3-node in such a way that 
 * the values in the node are in sorted order (from left to right).
 * Should only be used on filled up nodes.
 */
static void swapsort(float val, struct tree23_node * n)
{
	if (val > n->ldata && val <= n->rdata) {
		n->mdata = val;
	} else if (val <= n->ldata) {
		n->mdata = n->ldata;
		n->ldata = val;
	} else {
		n->mdata = n->rdata;
		n->rdata = val;
	}  
}

/*
 * Wraps a region of memory to write values to that is utilized by the
 * tree.
 * This might seem a little weird, but it's a simpler alternative
 * to emulating a class with a struct. Almost every call to free
 * and malloc is localized within this function.
 *
 * f: a flag that tells grabmem whether it needs to free the tree's
 * memory, fetch more memory, or clear a node and add it to the deleted
 * node buffer.
 * node_to_clear: A memory address that specifies the node to clear
 * and recycle.
 * Returns: a pointer to a node-sized region of memory, or NULL
 * if f is set to FREE.
 */
static struct tree23_node * modmem(fetch_style f, struct tree23_node * node_to_clear)
{
	static uint64_t buf_size = 8192; //Beginning size
	//The current memory buffer utilised by the program.
	static struct tree23_node * mem_buf = NULL;
	static uint64_t buf_ndx = 0; 
	/*
	 * Contains all memory buffers allocated by the program.
	 * This obviates the use of "realloc", which can render all
	 * tree node pointers useless.
	 */
	static struct tree23_node ** buffers = NULL;
	static uint64_t buffers_len = 8192;
	static uint64_t buffers_ndx = 0;
	/* 
	 * The buffer which stores pointers to nodes cleared by the rmval
	 * function.
	 */
	static struct tree23_node ** delbuf = NULL;
	static uint64_t delbuf_len = 8192;
	static uint64_t delbuf_ndx = 0;

	/* Initialize first-time use of mem_buf, as well as aux. buffers. */
	if (mem_buf == NULL) {
		mem_buf = malloc(sizeof(struct tree23_node) * buf_size);
		memset(mem_buf, '\0', sizeof(struct tree23_node) * buf_size);

		/*
		 * I am over-allocating a *lot* here, but that will mean far
		 * fewer reallocs for this array of node pointers.
		 */
		buffers = malloc(sizeof(struct tree23_node *) * buffers_len);
		buffers[0] = mem_buf;
		delbuf = malloc(sizeof(struct tree23_node *) * delbuf_len);
	}
	/* Index into the buffer that provides data to pointers. */
	if (f == GET) {
		/* Can't change the index after returning, so save the 
		 * old value.
		 */
		uint64_t temp = 0;//buf_ndx++;
		/* 
		 * Return a previously cleared node pointer if there are
		 * any left in the buffer filled with them.
		 */
		if (delbuf_ndx > 0) {
			return delbuf[--delbuf_ndx];
		}
		temp = buf_ndx++;
		if (buf_ndx > buf_size) {
			buf_size *= 2;
			mem_buf = malloc(sizeof(struct tree23_node) * buf_size);
			memset(mem_buf, '\0', sizeof(struct tree23_node) * 
							buf_size);
			/* Prefix increment used here because the first 
			 * element is always full.
			 */
			buffers[++buffers_ndx] = mem_buf;
			if (buffers_ndx == buffers_len) {
				buffers_len *= 2;
				/* Not going to bother using memset here, 
				 * as the memory is never read from before
				 * it's allocated.
				 */
				buffers = realloc(buffers, 
						sizeof(struct tree23_node *) * 
						buffers_len);
			}
			temp = 0;
			buf_ndx = 1;
		}
		return mem_buf + temp;
	} else if (f == DEL) {
		/*
	 	 * A call to rmval was made, clear up the passed in 
		 * address's data and add its address to the "free" buffer.
	 	 */
		if (node_to_clear == NULL) {
			fprintf(stderr, "Please pass in a valid address to "
					"clear.\n");
			return NULL; /* Perhaps ret a value other than NULL for 
			      * an error... */
		}
		delbuf[delbuf_ndx++] = node_to_clear;
		memset(node_to_clear, '\0', sizeof(struct tree23_node));
		if (delbuf_ndx == delbuf_len) {
			delbuf_len *= 2;
			delbuf = realloc(delbuf, 
				sizeof(struct tree23_node *) * delbuf_len);
		}
		return NULL;
	} else if (f == FREE) {
		/* Return everything to its initial state, free all buffers. */
		uint64_t i = 0;
		for (i; i <= buffers_ndx; i++) {
			memset(buffers[i], '\0', sizeof(struct tree23_node) * 
							8192 * (i + 1));
			free(buffers[i]);
		}
		memset(buffers, '\0', sizeof(struct tree23_node *) * 
								buffers_len);
		free(buffers);
		memset(delbuf, '\0', sizeof(struct tree23_node *) * delbuf_len);
		free(delbuf);
		buf_size = 8192;
		mem_buf = NULL;
		buf_ndx = 0;
		buffers = NULL;
		buffers_len = 8192;
		buffers_ndx = 0;
		delbuf = NULL;
		delbuf_len = 8192;
		delbuf_ndx = 0;
		return NULL;
	}
}

bool isvalid(struct tree23_node * curr)
{
	bool valid = true; //I'm feeling optimistic.
	struct tree23_node *parent = curr->parent;

	if (curr->left != NULL)
		valid = isvalid(curr->left);
	if (!valid)
		return valid;
	if (curr->ldata > curr->rdata && curr->is3node) {
		fprintf(stderr, "curr ldata: %d, rdata: %d\n",
			 curr->ldata, curr->rdata);
		fprintf(stderr, "Should never happen\n");
		return false;
	}

	switch(discern_childhood(curr, parent)) {
	case no_parent:
		break;
	case left:
		if (curr->ldata > parent->ldata)
			return false;
		if (curr->is3node && curr->rdata > parent->ldata)
			return false;
		if (parent->is3node) {
			if (curr->ldata > parent->rdata)
				return false;
			else if (curr->is3node && curr->rdata > parent->rdata)
   				return false;
		}
		break;
	case middle:
		if (curr->ldata < parent->ldata)
			return false;
		if (curr->ldata > parent->rdata)
			return false;
		if (curr->is3node) {
			if (curr->rdata < parent->ldata)
				return false;
			if (curr->rdata > parent->rdata)
				return false;
		}
		break;
	case right:
		if (curr->ldata < parent->ldata)
			return false;
		if (curr->is3node && curr->rdata < parent->ldata)
			return false;
		if (parent->is3node) {
			if (curr->ldata < parent->rdata)
				return false;
			else if (curr->is3node && curr->rdata < parent->rdata)
				return false;
		}
		break;
	}
	if (curr->middle != NULL)
		valid = isvalid(curr->middle);
	if (!valid) /* I think I need to check this a little more carefully. */
		return valid;
	if (curr->right != NULL)
		valid = isvalid(curr->right);
	return valid;
}
