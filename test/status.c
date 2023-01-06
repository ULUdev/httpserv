#include "http/status.h"
#include "include/test_status.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>
void test_numstatus(void **state) {
  httpserv_http_status_t status = 200;
  assert_true(status == HTTPSERV_HTTP_STATUS_OK);
}
void test_strstatus(void **state) {
  httpserv_http_status_t status = HTTPSERV_HTTP_STATUS_OK;
  assert_string_equal(httpserv_http_strstatus(status), "OK");
}
