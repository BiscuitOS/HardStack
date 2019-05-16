#include <stdio.h>
#include <stdlib.h>

/* header of 2-3-tree */
#include <tree23.h>

/* Embedd 2-3-tree */
struct node {
	struct tree23_node node;
	int idx;
};

#define node_entry(np) container_of(np, struct node, node)

/* Define 2-3-Tree root */
static struct tree23_root BiscuitOS_root = TREE23_ROOT_INIT;

/* Define node */
static struct node node0 = { .idx = 0x20 };

static int node_insert(struct tree23_root *root, struct node *node)
{
	struct tree23_node **new = &(root->node);

	/* Figure out where to put new node */
	if (*new) {
		while (*new) {
			printf("NEW\n");
		}
	} else {
		node->node.n = 1;
		node->node.
	}
	
	tree23_link_node(&node->node, new);
	tree23_insert(&node->node, root);
}

int main()
{
	/* Insert node into 2-3-tree */
	
	return 0;
}
