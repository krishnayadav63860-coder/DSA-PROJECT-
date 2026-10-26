#include "queue.h"

Queue* queue_create(void) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Failed to allocate queue");
        exit(EXIT_FAILURE);
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    return q;
}

void queue_enqueue(Queue *q, int data) {
    if (!q) return;
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        perror("Failed to allocate queue node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

int queue_dequeue(Queue *q) {
    if (!q || q->front == NULL) {
        return -1;
    }
    QueueNode *temp = q->front;
    int data = temp->data;

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return data;
}

bool queue_is_empty(const Queue *q) {
    return (q == NULL || q->front == NULL);
}

void queue_free(Queue *q) {
    if (!q) return;
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }
    free(q);
}
