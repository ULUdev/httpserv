#include "config.h"
#include "node.h"
#include "tree.h"
#include "include/test_config.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>

void config_load(void **state) {
  tree_t *cfg = httpserv_config_load("test/test.conf");
  assert_non_null(cfg);
  tree_node_t *node = tree_get_node(cfg, "http.some.value");
  assert_non_null(node);
  char *hello = malloc(6);
  strcpy(hello, "hello");
  assert_string_equal((char *)node->value, hello);
  free(hello);
  httpserv_config_destroy(cfg);
}
