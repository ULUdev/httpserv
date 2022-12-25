#include "logging.h"
#include "test_server.h"
#include "test_threadpool.h"
#include "test_string.h"
#include "test_config.h"
#include "http/method.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

int main(void) {
  httpserv_logging_init(NULL);
  /*
   * TODO: include more testing for the HTTP functions of libhttpserv
   */
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(tp_create),     cmocka_unit_test(tp_add_job),
      cmocka_unit_test(serv_create),   cmocka_unit_test(streq),
      cmocka_unit_test(strstriplw),    cmocka_unit_test(strstriptw),
      cmocka_unit_test(strstripltw),   cmocka_unit_test(config_load),
      cmocka_unit_test(method_to_str), cmocka_unit_test(str_to_method),
  };

  int res = cmocka_run_group_tests(tests, NULL, NULL);
  httpserv_logging_destroy();
  return res;
}
