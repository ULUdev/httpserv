#include "httpserver.h"
#include "test_server.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

void serv_create(void **state) {
  httpserv_httpserver_t *serv = httpserv_httpserver_new("127.0.0.1", 8080);
  assert_non_null(serv);
  httpserv_httpserver_destroy(serv);
}
