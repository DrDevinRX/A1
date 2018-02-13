#ifndef _QUEUE_H
#define _QUEUE_H

#include "proc.h"

typedef struct proc_queue_node_t
{
    proc p;
    proc_queue_node_t * next;
} queue_node;

typedef struct proc_queue_t
{
    queue_node * head;
    queue_node * tail;
    size_t size;
} queue;

queue * newQueue(proc p);
queue * newQueueAll(queue * q, proc * p, int size);
void add(queue * q, proc p);
proc removeProc(queue * q);
proc removeShortestBurst(queue * q);
void freeQueue(queue ** q);


#endif