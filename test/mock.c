#include "logging.h"
#include "test_server.h"
#include "test_threadpool.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

int main(void) {
  httpserv_logging_init("");
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(tp_create),
      cmocka_unit_test(tp_add_job),
      cmocka_unit_test(serv_create),
  };

  int res = cmocka_run_group_tests(tests, NULL, NULL);
  httpserv_logging_destroy();
  return res;
}
