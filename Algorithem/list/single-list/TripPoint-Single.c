/*
 * Single list demo code.
 *
 * (C) 2019.01.25 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

struct node {
    struct node *next;
    int num;
};

static void insert(struct node ***list, struct node *node)
{
    **list = node;
    *list  = &node->next;
}

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

    /* inert 1st node into list */
    insert(&node_list, &n0);

    /* inert 2st node into list */
    insert(&node_list, &n1);

    /* inert 3st into list */
    insert(&node_list, &n2);

    /* inert 4th node into list */
    insert(&node_list, &n3);


    /* Traverse all nodes */
    for (tmp = &n0; tmp; tmp = tmp->next)
        printf("%d\n", tmp->num);

    return 0;
}
