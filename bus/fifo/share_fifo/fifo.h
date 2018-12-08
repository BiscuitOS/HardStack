#ifndef _FIFO_H_
#define _FIFO_H_

/* Size must 16 Bytes */
struct node
{
    unsigned long index;
    unsigned long base;
    unsigned long size;
    struct node *next;
};

struct head 
{
    struct node *front;
    struct node *rear;
};

#endif
