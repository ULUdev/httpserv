#define _GNU_SOURCE
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

char *httpserv_time_str;
FILE *httpserv_log_file;

char *httpserv_logging_get_time() {
  time_t curtime = time(NULL);
  if (strftime(httpserv_time_str, HTTPSERV_LOGGING_TIME_LENGTH,
               HTTPSERV_LOGGING_TIME_FORMAT, localtime(&curtime)) == 0) {
    return "";
  } else {
    return httpserv_time_str;
  }
}

void httpserv_logging_init(const char *logfile) {
  httpserv_time_str = malloc(HTTPSERV_LOGGING_TIME_LENGTH);
  if (logfile) httpserv_log_file = fopen(logfile, "a");
  else httpserv_log_file = NULL;
}

void httpserv_logging_destroy() {
  free(httpserv_time_str);
  if (httpserv_log_file) fclose(httpserv_log_file);
}
void httpserv_logging_log(const char *msg, ...) {
  va_list ap;
  char *fmsg;
  char *time = httpserv_logging_get_time();
  va_start(ap, msg);
  vasprintf(&fmsg, msg, ap);
  fprintf(stderr, "[%s] %s\n", time, fmsg);
  if (httpserv_log_file) fprintf(httpserv_log_file, "[%s] %s\n", time, fmsg);
  free(fmsg);
}
void httpserv_logging_wrn(const char *msg, ...) {
  va_list ap;
  char *fmsg;
  char *time = httpserv_logging_get_time();
  va_start(ap, msg);
  vasprintf(&fmsg, msg, ap);
  fprintf(stderr, "[%s] Warning: %s\n", time, fmsg);
  if (httpserv_log_file) fprintf(httpserv_log_file, "[%s] Warning: %s\n", time, fmsg);
  free(fmsg);
}
void httpserv_logging_err(const char *msg, ...) {
  va_list ap;
  char *fmsg;
  char *time = httpserv_logging_get_time();
  va_start(ap, msg);
  vasprintf(&fmsg, msg, ap);
  fprintf(stderr, "[%s] Error: %s\n", time, fmsg);
  if (httpserv_log_file) fprintf(httpserv_log_file, "[%s] Error: %s\n", time, fmsg);
  free(fmsg);
}
