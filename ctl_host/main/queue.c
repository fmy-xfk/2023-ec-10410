#include "queue.h"

// 将Q清为空队列
void qClear(queue *Q) {
    Q->front = Q->rear = 0;
}
 
// 若队列Q为空队列，则返回1；否则返回-1
unsigned int qEmpty(queue *Q) {
    if (Q->front == Q->rear) // 队列空的标志
        return 1;
    else
        return 0;
}
 
// 返回Q的元素个数，即队列的长度
int qLength(queue *Q) {
    return (Q->rear - Q->front + MAX_QSIZE) % MAX_QSIZE;
}
 
// 若队列不空，则用e返回Q的队头元素，并返回1；否则返回0
unsigned int qFront(queue *Q, unsigned int *e) {
    if (Q->front == Q->rear) // 队列空
        return 0;
    *e = Q->base[Q->front];
    return 1;
}
 
// 插入元素e为Q的新的队尾元素
unsigned int qPush(queue *Q, unsigned int e) {
    if ((Q->rear + 1) % MAX_QSIZE == Q->front) // 队列满
        return 0;
    printf("%d\n",(int)e);
    Q->base[Q->rear] = e;
    Q->rear = (Q->rear + 1) % MAX_QSIZE;
    return 1;
}
 
// 若队列不空，则删除Q的队头元素，用e返回其值，并返回1；否则返回0
unsigned int qPoll(queue *Q, unsigned int *e) {
    if (Q->front == Q->rear) // 队列空
        return 0;
    *e = Q->base[Q->front];
    Q->front = (Q->front + 1) % MAX_QSIZE;
    return 1;
}

unsigned int qPop(queue *Q) {
    if (Q->front == Q->rear) // 队列空
        return 0;
    Q->front = (Q->front + 1) % MAX_QSIZE;
    return 1;
}

