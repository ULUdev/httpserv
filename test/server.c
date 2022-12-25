#include "net/server.h"
#include "include/test_server.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

void serv_create_v4(void **state) {
  httpserv_httpserver_t *serv = httpserv_httpserver_new(
      "127.0.0.1", 8080, HTTPSERV_HTTPSERVER_ADDR_KIND_IPv4);
  assert_non_null(serv);
  httpserv_httpserver_destroy(serv);
}
void serv_create_v6(void **state) {
  httpserv_httpserver_t *serv =
      httpserv_httpserver_new("::1", 8080, HTTPSERV_HTTPSERVER_ADDR_KIND_IPv6);
  assert_non_null(serv);
  httpserv_httpserver_destroy(serv);
}
