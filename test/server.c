#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <unistd.h>
#include "httpserver.h"
#include "logging.h"

static void serv_create(void **state) {
    httpserv_httpserver_t *serv = httpserv_httpserver_new("127.0.0.1", 8080);
    assert_non_null(serv);
    httpserv_httpserver_destroy(serv);
}

int main(void) {
    httpserv_logging_init("");
    const struct CMUnitTest test[] = {
        cmocka_unit_test(serv_create),
    };
    int res = cmocka_run_group_tests(test, NULL, NULL);
    httpserv_logging_destroy();
    return res;
}
