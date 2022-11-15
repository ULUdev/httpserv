#ifndef __THREADPOOL_JOB_H__
#define __THREADPOOL_JOB_H__
#include <pthread.h>

typedef struct ThreadpoolJob {
  void (*function)(void *arg);
  void *arg;
  struct ThreadpoolJob *next;
} threadpool_job_t;

typedef struct {
  threadpool_job_t *head;
  threadpool_job_t *tail;
  pthread_mutex_t *rwmutex;
  size_t size;
} threadpool_job_queue_t;

threadpool_job_t *threadpool_job_new(void (*function)(void *), void *arg);
int threadpool_job_exec(threadpool_job_t *job);
void threadpool_job_destroy(threadpool_job_t *job);

threadpool_job_queue_t *threadpool_job_queue_new();
void threadpool_job_queue_add_job(threadpool_job_queue_t *queue, threadpool_job_t *job);
threadpool_job_t *threadpool_job_queue_pop_job(threadpool_job_queue_t *queue);
void threadpool_job_queue_destroy(threadpool_job_queue_t *queue);
#endif
