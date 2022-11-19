#include "logging.h"
#include "threadpool.h"
#include <stdio.h>

// sketchy hack for unit testing. This avoids defining multiple main functions
#ifndef HTTPSERV_UNIT_TEST
int main(void) {
  // initalize logging system
  httpserv_logging_init("log.txt");
  httpserv_logging_log("creating threadpool...");
  threadpool_t *pool = threadpool_new(2);
  httpserv_logging_log("destroying threadpool...");
  threadpool_destroy(pool);
  // end logging system
  httpserv_logging_destroy();
  return 0;
}
#endif
