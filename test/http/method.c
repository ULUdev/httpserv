#include "http/request.h"
#include "method.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>
#include <string.h>

void method_to_str(void **state) {
  assert_string_equal(httpserv_http_strmethod(HTTPSERV_HTTP_METHOD_GET), "GET");
}
void str_to_method(void **state) {
  assert_true(httpserv_http_strtomethod("GET") == HTTPSERV_HTTP_METHOD_GET);
}
