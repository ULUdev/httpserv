#include "threadpool.h"
#include "job.h"
#include "logging.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define HTTPSERV_TP_WORKER_SLEEP_TIME 1

threadpool_t *threadpool_new(const size_t size) {
  if (size == 0)
    return NULL;
  threadpool_t *pool = malloc(sizeof(threadpool_t));
  pool->numthreads = size;
  pool->workers = malloc(size * sizeof(threadpool_worker_t));
  pool->queue = threadpool_job_queue_new();
  pool->mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(pool->mutex, NULL);
  pool->cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(pool->cond, NULL);
  for (size_t i = 0; i < size; i++) {
    pool->workers[i] = threadpool_worker_new(i, pool);
  }
  return pool;
}
threadpool_job_result_t *
threadpool_add_work(threadpool_t *pool, void *(*function)(void *), void *arg) {
  threadpool_job_t *job = threadpool_job_new(function, arg);
  pthread_mutex_lock(pool->mutex);
  threadpool_job_queue_add_job(pool->queue, job);
  pthread_cond_signal(pool->cond);
  pthread_mutex_unlock(pool->mutex);
  return job->jres;
}
void threadpool_ensure_jobs_done(threadpool_t *pool) {
  while (pool->queue->size > 0) {
  }
}
void threadpool_destroy(threadpool_t *pool) {
  for (int i = 0; i < pool->numthreads; i++) {
    threadpool_worker_stop(pool->workers[i]);
  }
  threadpool_job_queue_destroy(pool->queue);
  pthread_mutex_destroy(pool->mutex);
  pthread_cond_destroy(pool->cond);
  free(pool);
}

static void *threadpool_worker_func(void *arg) {
  threadpool_worker_data_t *data = (threadpool_worker_data_t *)arg;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  // threadpool_job_t *job = threadpool_job_queue_pop_job(data->pool->queue);
  while (1) {
    pthread_mutex_lock(data->pool->mutex);
    threadpool_job_t *job = threadpool_job_queue_pop_job(data->pool->queue);
    if (!job) {
      pthread_cond_wait(data->pool->cond, data->pool->mutex);
      job = threadpool_job_queue_pop_job(data->pool->queue);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_unlock(data->pool->mutex);
    if (job) {
      httpserv_logging_log("Worker %zu executing job...", data->worker->id);
      threadpool_job_exec(job);
      if (job->jres->done == -1) {
        httpserv_logging_wrn("Worker %zu: job failed!", data->worker->id);
      }
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  }
  return NULL;
}

threadpool_worker_t *threadpool_worker_new(size_t id, threadpool_t *pool) {
  threadpool_worker_t *worker = malloc(sizeof(threadpool_worker_t));
  worker->id = id;
  worker->thread = malloc(sizeof(pthread_t));
  pthread_create(worker->thread, NULL, &threadpool_worker_func,
                 (void *)threadpool_worker_data_new(worker, pool));
  pthread_detach(*worker->thread);
  return worker;
}
void threadpool_worker_stop(threadpool_worker_t *worker) {
  pthread_cancel(*worker->thread);
  pthread_join(*worker->thread, NULL);
  free(worker);
}
threadpool_worker_data_t *
threadpool_worker_data_new(threadpool_worker_t *worker, threadpool_t *pool) {
  threadpool_worker_data_t *data = malloc(sizeof(threadpool_worker_data_t));
  data->worker = worker;
  data->pool = pool;
  return data;
}
void threadpool_worker_data_destroy(threadpool_worker_data_t *data) {
  free(data);
}
