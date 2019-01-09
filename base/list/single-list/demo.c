#include <stdio.h>
#include <stdlib.h>

struct node {
    struct node *next;
    int num;
};

int main()
{
    static struct node n0, n1, n2, n3;
    struct node **node_list = NULL;
    struct node *tmp;
    
    n0.num = 0x0;
    n1.num = 0x1;
    n2.num = 0x2;
    n3.num = 0x3;

    /* node list init */
    node_list = &n0.next;

    /* Insert 1st node */
    *node_list = &n0;
    node_list = &n0.next;

    /* Insert 2st node */
    *node_list = &n1;
    node_list = &n1.next;

    /* Insert 3th node */
    *node_list = &n2;
    node_list = &n2.next;

    /* Insert 4th node */
    *node_list = &n3;
    node_list = &n3.next;

    /* Traverse all nodes */
    for (tmp = &n0; tmp; tmp = tmp->next)
        printf("%d\n", tmp->num);

    return 0;
}
