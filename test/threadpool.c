#include "threadpool.h"
#include "job.h"
#include "logging.h"
#include "test_threadpool.h"
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
  assert_null(arg);
  return "passed";
}

void tp_add_job(void **state) {
  threadpool_t *pool = threadpool_new(3);

  threadpool_job_result_t *jr1 = threadpool_add_work(pool, test_work, NULL);
  threadpool_job_result_t *jr2 = threadpool_add_work(pool, test_work, NULL);
  threadpool_job_result_t *jr3 = threadpool_add_work(pool, test_work, NULL);

  threadpool_job_result_await(jr1);
  threadpool_job_result_await(jr2);
  threadpool_job_result_await(jr3);

  assert_true(jr1->done == 0);
  assert_true(jr2->done == 0);
  assert_true(jr3->done == 0);

  assert_string_equal(jr1->res, "passed");
  assert_string_equal(jr2->res, "passed");
  assert_string_equal(jr3->res, "passed");

  threadpool_job_result_destroy(jr1);
  threadpool_job_result_destroy(jr2);
  threadpool_job_result_destroy(jr3);

  threadpool_destroy(pool);
}
