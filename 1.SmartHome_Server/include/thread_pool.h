/*************************************************************************
	> File Name: thread_pool.h
	> Author: 
	> Mail: 
	> Created Time: Thu 18 Jan 2024 10:00:27 PM CST
 ************************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#include "datatype.h"
#include "server_recv.h"
struct task_queue {
    int size;
    int total;
    int head;
    int tail;
    struct Link_Args **data;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void task_queue_init(struct task_queue *taskQueue, int size);
void task_queue_push(struct task_queue *task_queue, struct Link_Args *);
struct Link_Args *task_queue_pop(struct task_queue *task_queue);



#endif
