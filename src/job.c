#include "job.h"
#include "logging.h"
#include <asm-generic/errno-base.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

threadpool_job_result_t *threadpool_job_result_new() {
  threadpool_job_result_t *jres = malloc(sizeof(threadpool_job_result_t));
  jres->done = 1;
  jres->mutex = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(jres->mutex, NULL) != 0) {
    httpserv_logging_err("creation of mutex failed");
    return NULL;
  }
  jres->res = NULL;
  return jres;
}
void threadpool_job_result_set(threadpool_job_result_t *jres, void *res) {
  pthread_mutex_lock(jres->mutex);
  jres->res = res;
  pthread_mutex_unlock(jres->mutex);
}
void *threadpool_job_result_await(threadpool_job_result_t *jres) {
  while (jres->done > 0) {
  }
  pthread_mutex_lock(jres->mutex);
  return jres->res;
}
void threadpool_job_result_destroy(threadpool_job_result_t *jres) {
  pthread_mutex_destroy(jres->mutex);
  free(jres->mutex);
  free(jres);
}

threadpool_job_t *threadpool_job_new(void *(*function)(void *), void *arg) {
  threadpool_job_t *job = malloc(sizeof(threadpool_job_t));
  job->function = function;
  job->arg = arg;
  job->next = NULL;
  job->jres = threadpool_job_result_new();
  return job;
}
void *threadpool_job_exec(threadpool_job_t *job) {
  if (!job->function) {
    pthread_mutex_lock(job->jres->mutex);
    job->jres->done = -1;
    pthread_mutex_unlock(job->jres->mutex);
    return NULL;
  }
  threadpool_job_result_set(job->jres, job->function(job->arg));
  job->jres->done = 0;
  return job->jres->res;
}
void threadpool_job_destroy(threadpool_job_t *job) { free(job); }

threadpool_job_queue_t *threadpool_job_queue_new() {
  threadpool_job_queue_t *queue = malloc(sizeof(threadpool_job_queue_t));
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  queue->rwmutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(queue->rwmutex, NULL);
  queue->cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(queue->cond, NULL);
  return queue;
}
void threadpool_job_queue_add_job(threadpool_job_queue_t *queue,
                                  threadpool_job_t *job) {
  pthread_mutex_lock(queue->rwmutex);
  if (!queue->head) {
    queue->tail = job;
    queue->head = job;
  } else {
    job->next = NULL;
    queue->tail->next = job;
    queue->tail = job;
  }
  queue->size++;
  pthread_cond_signal(queue->cond);
  pthread_mutex_unlock(queue->rwmutex);
}
threadpool_job_t *threadpool_job_queue_pop_job(threadpool_job_queue_t *queue) {
  pthread_mutex_lock(queue->rwmutex);
  if (queue->size == 0) {
    pthread_mutex_unlock(queue->rwmutex);
    return NULL;
  } else {
    threadpool_job_t *job = queue->head;
    queue->head = job->next;
    job->next = NULL;
    queue->size--;
    pthread_mutex_unlock(queue->rwmutex);
    return job;
  }
}
void threadpool_job_queue_destroy(threadpool_job_queue_t *queue) {
  pthread_mutex_lock(queue->rwmutex);
  threadpool_job_t *cur = queue->head;
  for (int i = 0; i < queue->size; i++) {
    threadpool_job_t *next = cur->next;
    threadpool_job_destroy(cur);
    cur = next;
  }
  pthread_mutex_unlock(queue->rwmutex);
  pthread_mutex_destroy(queue->rwmutex);
  free(queue);
}
