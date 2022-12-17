#ifndef __HTTPSERV_HTTP_REQUEST_H__
#define __HTTPSERV_HTTP_REQUEST_H__
#include <stdlib.h>
#include "data.h"
/*
 * An enum defining the HTTP methods this server supports
 */
typedef enum {
  HTTPSERV_HTTP_METHOD_GET,
  HTTPSERV_HTTP_METHOD_POST,
  HTTPSERV_HTTP_METHOD_PUT,
  HTTPSERV_HTTP_METHOD_DELETE,
  HTTPSERV_HTTP_METHOD_CONNECT,
  HTTPSERV_HTTP_METHOD_OPTIONS,
  HTTPSERV_HTTP_METHOD_TRACE,
  HTTPSERV_HTTP_METHOD_PATCH,
  HTTPSERV_HTTP_METHOD_INVALID,
} httpserv_http_method_t;

char *httpserv_http_strmethod(const httpserv_http_method_t m);
httpserv_http_method_t httpserv_http_strtomethod(const char *s);

typedef struct {
  httpserv_http_method_t method;
  char *path;
  char **headers;
  size_t numheaders;
  char *body;
  size_t content_length;
} httpserv_http_request_t;

httpserv_http_request_t *
httpserv_http_request_new(const httpserv_http_method_t method,
                          const char *path);
httpserv_http_request_t *
httpserv_http_request_from_raw(httpserv_raw_data_t *raw);
void httpserv_http_request_set_body(httpserv_http_request_t *req,
                                    const char *body, size_t len);
void httpserv_http_request_add_header(httpserv_http_request_t *req,
                                      const char *key, const char *value);
void httpserv_http_request_destroy(httpserv_http_request_t *req);
char *httpserv_http_request_get_header(const httpserv_http_request_t *req,
                                       const char *key);

httpserv_raw_data_t *
httpserv_http_request_build(const httpserv_http_request_t *req);

#endif
