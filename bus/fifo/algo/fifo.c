#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

struct node
{
    int data;
    unsigned long base;
    unsigned long size;
    struct node *next;
};

struct head 
{
    struct node *front;
    struct node *rear;
};

/*
 * Initialize FIFO queue
 */
int InitLinkQueue(struct head *LQ)
{
    struct node *p;

    p = (struct node *)malloc(sizeof(struct node));
    if (p == NULL)
        return -1;
    else {
        p->next = NULL;
        LQ->front = p;
        LQ->rear = p;
        return 0;
    }
}

/*
 * Verify FIFO is empty
 */
int IsQueueEmpty(struct head *LQ)
{
    if (LQ->front == LQ->rear)
        return 1;
    else
        return 0;
}

/*
 * Push new node into FIFO
 */
int PushElement(struct head *LQ, int dData)
{
    struct node *p;

    p = (struct node *)malloc(sizeof(struct node));
    if (p == NULL)
        return -1;

    p->data = dData;
    p->next = NULL;
    LQ->rear->next = p;
    LQ->rear = p;

    return 0;
}

/*
 * Pop node from FIFO
 */
int PopElement(struct head *pLQ, int *pData)
{
    struct node *p;
    if (IsQueueEmpty(pLQ))
        return -1;

    p = pLQ->front->next;
    *pData = p->data;
    pLQ->front->next = p->next;
    if (pLQ->front->next == NULL)
        pLQ->rear = pLQ->front;

    free(p);

    return 0;
}

/* 
 * Obtain data from FIFO head node.
 */
int GetHeadElement(struct head *pLQ, int *pData)
{
    if (IsQueueEmpty(pLQ))
        return -1;

    *pData = pLQ->front->next->data;

    return 0;
}

static int usage()
{
    /* FIFO Queue */
    struct head FIFO;
    int i;
    
    /* Initialize FIFO queue */
    if(InitLinkQueue(&FIFO) < 0) {
        printf("Failed to init FIFO.\n");
        return -1;
    }

    for (i = 0; i < 20; i++) {
        if (PushElement(&FIFO, i) < 0) {
            printf("Unable push data into FIFO.\n");
            return -1;
        }
    }

    while (!IsQueueEmpty(&FIFO)) {
        int value;

        PopElement(&FIFO, &value);
        printf("%d\n", value);
    }

    printf("Hello World\n");

    return 0;
}
