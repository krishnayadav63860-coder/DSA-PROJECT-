#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

/* Node for linked-list queue */
typedef struct QueueNode {
    int data;
    struct QueueNode *next;
} QueueNode;

/* FIFO Queue structure with front and rear pointers */
typedef struct Queue {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

/* Queue API */
Queue* queue_create(void);
void queue_enqueue(Queue *q, int data);
int queue_dequeue(Queue *q);
bool queue_is_empty(const Queue *q);
void queue_free(Queue *q);

#endif /* QUEUE_H */
