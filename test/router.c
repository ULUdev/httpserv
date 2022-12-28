#include "include/test_router.h"
#include "router.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <cmocka.h>

void route_create(void **state) {
  httpserv_route_t *route =
      httpserv_route_new("", "", HTTPSERV_ROUTE_KIND_ALIAS);
  assert_non_null(route);
  assert_true(route->kind = HTTPSERV_ROUTE_KIND_ALIAS);
  httpserv_route_destroy(route);
}
void router_create(void **state) {
  httpserv_router_t *router = httpserv_router_new();
  httpserv_router_destroy(router);
}
void router_route(void **state) {
  httpserv_router_t *router = httpserv_router_new();
  httpserv_route_t *route =
      httpserv_route_new(".*", "/", HTTPSERV_ROUTE_KIND_ALIAS);
  httpserv_router_add_route(router, route);
  assert_ptr_equal(route, httpserv_router_lookup(router, "/hello"));
  httpserv_router_destroy(router);
}
