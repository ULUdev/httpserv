#include <stdio.h>
#include "logging.h"
#include "threadpool.h"

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
