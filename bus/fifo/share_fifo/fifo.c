#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "fifo.h"

#define TOTAL_SIZES              (6 * 1024 * 1024)
#define NODE_NUM                 (256)
#define NODE_SIZES               ((NODE_NUM + 1) * sizeof(struct node))
#define EFLAGS_BYTES             (4)
#define MAGIC_BYTES              (4)
#define HEAD_BYTES               (sizeof(struct head))
#define BITMAP_BYTES             ((NODE_NUM + 7) / 8)
#define EFLAGS_OFFSET            (0)
#define MAGIC_OFFSET             (EFLAGS_OFFSET + EFLAGS_BYTES)
#define HEAD_OFFSET              (MAGIC_OFFSET + MAGIC_BYTES)
#define BITMAP_OFFSET            (HEAD_OFFSET + HEAD_BYTES)
#define NODE_OFFSET              (BITMAP_OFFSET + BITMAP_BYTES)
#define RESERVED_SIZES           (NODE_OFFSET + NODE_SIZES)
#define MEM_OFFSET               (RESERVED_SIZES)
#define MEM_SIZES                (TOTAL_SIZES - MEM_OFFSET)
#define FIFO_MAGIC               0x91929400
/*
 * 
 * +--------+------+--------+--------------------------------------+
 * |        |      |        |                                      |
 * | EFLAGS | Head | BitMap |             Nodes Memory             |
 * |        |      |        |                                      |
 * +--------+------+--------+--------------------------------------+
 *
 */

unsigned char *bitmap_node = NULL;
struct node *node_list = NULL;
struct head *fifo_head = NULL;
unsigned int *fifo_eflags = NULL;
unsigned int *fifo_magic = NULL;

/*
 * Obtain a free node index.
 */
static int bitmap_alloc(void)
{
    int i, j;

    if (bitmap_node == NULL)
        return -1;

    for (i = 0; i < BITMAP_BYTES; i++) {
        for (j = 0; j < 8; j++) {
            if (!((bitmap_node[i] >> j) & 0x1)) {
                bitmap_node[i] |= (1 << j);
                return (i * 8 + j);
            }
        }
    }
    return -1;
}

/*
 * Release a unused node by index.
 */
static int bitmap_free(int offset)
{
    int i, j;

    if (bitmap_node == NULL)
        return -1;

    if (offset >= (BITMAP_BYTES * 8))
        return -1;

    i = offset / 8;
    j = offset % 8;

    bitmap_node[i] &= ~(1 << j);

    return 0;
}

/*
 * Initaizlie fifo .....
 */
static int fifo_init(unsigned long base)
{
    /* Clear all Reserved memory */
    memset((unsigned long *)base, 0, RESERVED_SIZES);
    bitmap_node = (unsigned char *)(base + BITMAP_OFFSET);
    node_list = (struct node *)(base + NODE_OFFSET);
    fifo_eflags = (unsigned int *)(base + EFLAGS_OFFSET);
    fifo_magic = (unsigned int *)(base + MAGIC_OFFSET);
    *fifo_magic = FIFO_MAGIC;
    fifo_head = (struct head *)(base + HEAD_OFFSET);
    fifo_head->front = NULL;
    fifo_head->rear = NULL;

    return 0;
}

/*
 * Allocate new node
 */
static struct node *alloc_node(void)
{
    int index;
    struct node *node;

    index = bitmap_alloc();
    if (index < 0)
        return NULL;
    node = node_list + index;

    memset(node, 0, sizeof(struct node));

    node->index = index;
    return node;
}

/* Free node */
static void free_node(struct node *node)
{
    bitmap_free(node->index);
}

int InitLinkQueue(void)
{
    struct node *p;

    p = alloc_node();
    if (p == NULL)
        return -1;
    else {
        p->base = MEM_OFFSET;
        p->size = 0;
        p->next = NULL;
        fifo_head->front = p;
        fifo_head->rear = p;
        return 0;
    }
}

/*
 * Verify FIFO is empty
 */
int IsQueueEmpty(void)
{
    if (fifo_head->front == fifo_head->rear)
        return 1;
    else
        return 0;
}

/*
 * Push new node into FIFO
 */
int PushElement(unsigned long base, unsigned long size)
{
    struct node *p;

    p = alloc_node();
    if (p == NULL)
        return -1;

    p->base = base;
    p->size = size;
    p->next = NULL;
    fifo_head->rear->next = p;
    fifo_head->rear = p;

    return 0;
}

/*
 * Pop node from FIFO
 */
int PopElement(unsigned long *pbase, unsigned long *psize)
{
    struct node *p;

    if (IsQueueEmpty())
        return -1;

    p = fifo_head->front->next;
    *pbase = p->base;
    *psize = p->size;
    fifo_head->front->next = p->next;
    if (fifo_head->front->next == NULL)
        fifo_head->rear = fifo_head->front;

    free_node(p);

    return 0;
}

/* 
 * Obtain data from FIFO head node.
 */
int GetHeadElement(unsigned long *pbase, unsigned long *psize)
{
    if (IsQueueEmpty()) {
        *pbase = MEM_OFFSET;
        *psize = 0;
        return 0;
    }

    *pbase = fifo_head->rear->base;
    *psize = fifo_head->rear->size;

    return 0;
}


int main(void)
{
    char *p;
    int i;

    p = malloc(4096);

    printf("Initialize.....\n");
    fifo_init((unsigned long)p);

    /* Initialize FIFO queue */
    if(InitLinkQueue() < 0) {
        printf("Failed to init FIFO.\n");
        return -1;
    }

    for (i = 0; i < 20; i++) {
        unsigned long base, size;

        GetHeadElement(&base, &size);
        printf("=>Base: %d Size: %d\n", base, size);
        if (PushElement(base + size, 100) < 0) {
            printf("Unable push data into FIFO.\n");
            return -1;
        }
    }

    while (!IsQueueEmpty()) {
        unsigned long base, size;

        PopElement(&base, &size);
        printf("Base: %d Size: %d\n", base, size);
    }

    printf("Hello World\n");


    return 0;
}
