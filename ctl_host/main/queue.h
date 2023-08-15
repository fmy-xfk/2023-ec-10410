#ifndef __QUEUE_H__
#define __QUEUE_H__

// 队列的顺序存储结构(循环队列)
#define MAX_QSIZE 8 // 最大队列长度+1
typedef struct {
    unsigned int base[MAX_QSIZE]; // 初始化的动态分配存储空间
    int front; // 头指针，若队列不空，指向队列头元素
    int rear; // 尾指针，若队列不空，指向队列尾元素的下一个位置
} queue;
 
// 将Q清为空队列
void qClear(queue *Q);

// 若队列Q为空队列，则返回1；否则返回-1
unsigned int qEmpty(queue *Q);

// 返回Q的元素个数，即队列的长度
int qLength(queue *Q);
 
// 若队列不空，则用e返回Q的队头元素，并返回1；否则返回0
unsigned int qFront(queue *Q,unsigned int *e);
 
// 插入元素e为Q的新的队尾元素
unsigned int qPush(queue *Q,unsigned int e);
 
// 若队列不空，则删除Q的队头元素，用e返回其值，并返回1；否则返回0
unsigned int qPoll(queue *Q,unsigned int *e);

unsigned int qPop(queue *Q);

#endif