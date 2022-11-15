#include "logging.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char *httpserv_time_str;
//FILE *httpserv_log_file;

char *httpserv_logging_get_time() {
  time_t curtime = time(NULL);
  if (strftime(httpserv_time_str, HTTPSERV_LOGGING_TIME_LENGTH, HTTPSERV_LOGGING_TIME_FORMAT, localtime(&curtime))==0) {
    return "";
  } else {
    return httpserv_time_str;
  }
}

void httpserv_logging_init(const char *logfile) {
  httpserv_time_str = malloc(HTTPSERV_LOGGING_TIME_LENGTH);
  //httpserv_log_file = fopen(logfile, "a");
}

void httpserv_logging_destroy() {
  free(httpserv_time_str);
  //fclose(httpserv_log_file);
}
