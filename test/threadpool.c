#include "threadpool.h"
#include "job.h"
#include "logging.h"
#include "include/test_threadpool.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

void tp_create(void **state) {
  threadpool_t *pool = threadpool_new(1);
  assert_non_null(pool);
  threadpool_destroy(pool);
}

void *test_work(void *arg) {
  assert_non_null(arg);
  return "passed";
}

void tp_add_job(void **state) {
  threadpool_t *pool = threadpool_new(3);

  int *jobnum;
  jobnum = malloc(sizeof(int));
  *jobnum = 1;
  threadpool_job_result_t *jr1 = threadpool_add_work(pool, test_work, jobnum);
  threadpool_job_result_t *jr2 = threadpool_add_work(pool, test_work, jobnum);
  threadpool_job_result_t *jr3 = threadpool_add_work(pool, test_work, jobnum);

  // function blocks on these calls
  threadpool_job_result_await(jr1);
  threadpool_job_result_await(jr2);
  threadpool_job_result_await(jr3);

  // make sure that all jobs are done and they didn't fail
  assert_true(jr1->done == 0);
  assert_true(jr2->done == 0);
  assert_true(jr3->done == 0);

  // make sure all jobs returned the correct value
  assert_string_equal(jr1->res, "passed");
  httpserv_logging_log("job 1 passed");
  assert_string_equal(jr2->res, "passed");
  httpserv_logging_log("job 2 passed");
  assert_string_equal(jr3->res, "passed");
  httpserv_logging_log("job 3 passed");

  threadpool_job_result_destroy(jr1);
  threadpool_job_result_destroy(jr2);
  threadpool_job_result_destroy(jr3);

  threadpool_destroy(pool);
  free(jobnum);
}
