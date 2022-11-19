#include "job.h"
#include <pthread.h>
#include <stdlib.h>

threadpool_job_t *threadpool_job_new(void (*function)(void *), void *arg) {
  threadpool_job_t *job = malloc(sizeof(threadpool_job_t));
  job->function = function;
  job->arg = arg;
  job->next = NULL;
  return job;
}
int threadpool_job_exec(threadpool_job_t *job) {
  if (!job->function)
    return 1;
  job->function(job->arg);
  return 0;
}
void threadpool_job_destroy(threadpool_job_t *job) { free(job); }

threadpool_job_queue_t *threadpool_job_queue_new() {
  threadpool_job_queue_t *queue = malloc(sizeof(threadpool_job_queue_t));
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  queue->rwmutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(queue->rwmutex, NULL);
  return queue;
}
void threadpool_job_queue_add_job(threadpool_job_queue_t *queue,
                                  threadpool_job_t *job) {
  if (!queue->head) {
    queue->tail = job;
    queue->head = job;
  } else {
    job->next = NULL;
    queue->tail->next = job;
    queue->tail = job;
  }
  queue->size++;
}
threadpool_job_t *threadpool_job_queue_pop_job(threadpool_job_queue_t *queue) {
  if (queue->size == 0) {
    return NULL;
  } else {
    threadpool_job_t *job = queue->head;
    queue->head = job->next;
    job->next = NULL;
    queue->size--;
    return job;
  }
}
void threadpool_job_queue_destroy(threadpool_job_queue_t *queue) {
  threadpool_job_t *cur = queue->head;
  for (int i = 0; i < queue->size; i++) {
    threadpool_job_t *next = cur->next;
    threadpool_job_destroy(cur);
    cur = next;
  }
  pthread_mutex_destroy(queue->rwmutex);
  free(queue);
}
