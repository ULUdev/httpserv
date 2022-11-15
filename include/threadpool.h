#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include <pthread.h>
#include <stdlib.h>
#include "job.h"

typedef struct {
  size_t id;
  pthread_t *thread;
} threadpool_worker_t;

typedef struct {
  threadpool_worker_t **workers;
  size_t numthreads;
  threadpool_job_queue_t *queue;
} threadpool_t;

typedef struct {
  threadpool_worker_t *worker;
  threadpool_t *pool;
} threadpool_worker_data_t;

threadpool_t *threadpool_new(const size_t size);
void threadpool_add_work(threadpool_t *pool, void (*function)(void *), void *arg);
void threadpool_destroy(threadpool_t *pool);

threadpool_worker_t *threadpool_worker_new(size_t id, threadpool_t *pool);
void threadpool_worker_stop(threadpool_worker_t *worker);

threadpool_worker_data_t *threadpool_worker_data_new(threadpool_worker_t *worker, threadpool_t *pool);
void threadpool_worker_data_destroy(threadpool_worker_data_t *data);

#endif
