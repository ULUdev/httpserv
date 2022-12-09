#ifndef __HTTPSERV_LOGGING_H__
#define __HTTPSERV_LOGGING_H__
#include <stdio.h>

extern char *httpserv_time_str;
extern FILE *httpserv_log_file;

#define HTTPSERV_LOGGING_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
/*
 * Note: Always define this as the maximum length producable by
 * HTTPSERV_LOGGING_TIME_FORMAT or at least large enough to hold that
 */
#define HTTPSERV_LOGGING_TIME_LENGTH 20

/*
  #define httpserv_logging_log(msg, ...)				\
  fprintf(stderr, "[%s:%d %s] " msg "\n", __FILE__, __LINE__,                  \
          httpserv_logging_get_time(), ##__VA_ARGS__);                         \
  if (httpserv_log_file)                                                       \
  fprintf(httpserv_log_file, "[%s:%d %s] " msg "\n", __FILE__, __LINE__,       \
          httpserv_logging_get_time(), ##__VA_ARGS__)
#define httpserv_logging_wrn(msg, ...)                                         \
  fprintf(stderr, "[%s:%d %s] Warning: " msg "\n", __FILE__, __LINE__,         \
          httpserv_logging_get_time(), ##__VA_ARGS__);                         \
  if (httpserv_log_file)                                                       \
  fprintf(httpserv_log_file, "[%s:%d %s] Warning: " msg "\n", __FILE__,        \
          __LINE__, httpserv_logging_get_time(), ##__VA_ARGS__)
#define httpserv_logging_err(msg, ...)                                         \
  fprintf(stderr, "[%s:%d %s] Error: " msg "\n", __FILE__, __LINE__,           \
          httpserv_logging_get_time(), ##__VA_ARGS__);                         \
  if (httpserv_log_file)                                                       \
    fprintf(httpserv_log_file, "[%s:%d %s] Error: " msg "\n", __FILE__,        \
            __LINE__, httpserv_logging_get_time(), ##__VA_ARGS__);
*/

char *httpserv_logging_get_time();
void httpserv_logging_init(const char *logfile);
void httpserv_logging_destroy();
void httpserv_logging_log(const char *msg, ...);
void httpserv_logging_wrn(const char *msg, ...);
void httpserv_logging_err(const char *msg, ...);

#endif
