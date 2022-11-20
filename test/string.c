#include "stringh.h"
#include "test_string.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>

void streq(void **state) {
  assert_true(httpserv_streq("abc", "abc") == 0);
  assert_false(httpserv_streq("abc", "def") == 0);
}

void strstriplw(void **state) {
  char *s = malloc(6);
  strcpy(s, "  abc");
  httpserv_strstriplw(s);
  assert_string_equal(s, "abc");
  free(s);
}
void strstriptw(void **state) {
  char *s = malloc(6);
  strcpy(s, "abc  ");
  httpserv_strstriptw(s);
  assert_string_equal(s, "abc");
  free(s);
}
void strstripltw(void **state) {
  char *s = malloc(8);
  strcpy(s, "  abc  ");
  httpserv_strstripltw(s);
  assert_string_equal(s, "abc");
  free(s);
}
