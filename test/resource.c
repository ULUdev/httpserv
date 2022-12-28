#include "resource.h"
#include "include/test_resource.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

void resource_load(void **state) {
  httpserv_resource_t *res = httpserv_resource_new("test/test.html");
  assert_non_null(res);
  httpserv_resource_destroy(res);
}
