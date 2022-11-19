#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <unistd.h>
#include "logging.h"
#include "threadpool.h"

static void tp_create(void **state) {
    threadpool_t *pool = threadpool_new(1);
    assert_non_null(pool);
    threadpool_destroy(pool);
}

void test_work(void *arg) {
    assert_null(arg);
    sleep(1);
}

static void tp_add_job(void **state) {
    threadpool_t *pool = threadpool_new(2);
    threadpool_add_work(pool, test_work, NULL);
    threadpool_add_work(pool, test_work, NULL);
    threadpool_add_work(pool, test_work, NULL);
    threadpool_destroy(pool);
}

int main(void) {
    httpserv_logging_init("");
    const struct CMUnitTest test[] = {
        cmocka_unit_test(tp_create),
        cmocka_unit_test(tp_add_job),
    };
    int res = cmocka_run_group_tests(test, NULL, NULL);
    httpserv_logging_destroy();
    return res;
}
