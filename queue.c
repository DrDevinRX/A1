#include "queue.h"

queue * newQueue(proc p)
{
    queue * ptr = malloc(sizeof(queue));
    queue_node * q = malloc(sizeof(queue_node));

    if (!q || !ptr) return NULL;

    q->next = NULL;
    q->p = p;

    ptr->head = q = ptr->tail;
    ptr->size = 0;
    
    return ptr;
}

void addOne(queue *q, proc p)
{
    queue_node * c = q->head;
    while (c != NULL)
        c = c->next;
    c = malloc(sizeof(queue));
    c->p = p;
    q->tail = c;
    (q->size)++;
}

proc removeProc(queue * q)
{
    queue_node * c = q->head;

    if (!c)
    {
        proc pBad = {.burst = 100000};
        return pBad;
    }

    proc p = c->p;

    q->head = c->next;
    (q->size)--;
    free(c);

    return p;
}

proc removeShortestBurst(queue * q)
{
    queue_node * c = q->head;
    proc p, p1, p2;

    while (q->size)
    {
        p1 = removeProc(q);
        p2 = removeProc(q);
    }

    return p;
}

void freeQueue(queue **q)
{
    queue * c = *q;
    while (c != NULL)
    {

    }

}