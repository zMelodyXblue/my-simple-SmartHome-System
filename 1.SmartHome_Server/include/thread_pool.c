/*************************************************************************
	> File Name: thread_pool.c
	> Author: 
	> Mail: 
	> Created Time: Thu 18 Jan 2024 10:08:48 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <head.h>
#include <pthread.h>
#include <thread_pool.h>

void task_queue_init(struct task_queue *taskQueue, int size) {
    taskQueue->size = size;
    taskQueue->total = 0;
    taskQueue->head = 0;
    taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
    taskQueue->data = (struct Link_Args **)calloc(size, sizeof(struct Link_Args *));
    return ;
}

void task_queue_push(struct task_queue *taskQueue, struct Link_Args *clientLink) {
    pthread_mutex_lock(&taskQueue->mutex);
    if (taskQueue->total == taskQueue->size) {
        DBG(RED"taskQueue is full!\n"NONE);
        pthread_mutex_unlock(&taskQueue->mutex);
        return ;  //什么也不做，类似于丢包
    }
    taskQueue->data[taskQueue->tail] = clientLink;
    taskQueue->total++;
    if (++taskQueue->tail == taskQueue->size) {
        taskQueue->tail = 0;
    }
    DBG(GREEN"<fd=%d, tasktype=%d> data is pushed to taskQueue!"NONE "\n", clientLink->sockfd, clientLink->tasktype);
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}

struct Link_Args *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->total == 0) {
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct Link_Args *clientLink = taskQueue->data[taskQueue->head];
    taskQueue->total--;
    DBG(GREEN"<D> taskQueue pop: %d\n"NONE, clientLink->sockfd);
    if (++taskQueue->head == taskQueue->size) {
        DBG(RED"<D> taskQueue touched tail!\n"NONE);
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return clientLink;
}

