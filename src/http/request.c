#include "http/request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "stringh.h"
// test if two strings are equal. NOTE: this requires
#define streq(s1, s2) (httpserv_streq(s1, s2) == 0)

char *httpserv_http_strmethod(const httpserv_http_method_t m) {
  switch (m) {
  case HTTPSERV_HTTP_METHOD_GET:
    return "GET";
  case HTTPSERV_HTTP_METHOD_POST:
    return "POST";
  case HTTPSERV_HTTP_METHOD_PUT:
    return "PUT";
  case HTTPSERV_HTTP_METHOD_DELETE:
    return "DELETE";
  case HTTPSERV_HTTP_METHOD_CONNECT:
    return "CONNECT";
  case HTTPSERV_HTTP_METHOD_OPTIONS:
    return "OPTIONS";
  case HTTPSERV_HTTP_METHOD_TRACE:
    return "TRACE";
  case HTTPSERV_HTTP_METHOD_PATCH:
    return "PATCH";
  default:
    return NULL;
  }
}
httpserv_http_method_t httpserv_http_strtomethod(const char *s) {
  if (streq(s, "GET")) return HTTPSERV_HTTP_METHOD_GET;
  else if (streq(s, "POST")) return HTTPSERV_HTTP_METHOD_POST;
  else if (streq(s, "PUT")) return HTTPSERV_HTTP_METHOD_PUT;
  else if (streq(s, "DELETE")) return HTTPSERV_HTTP_METHOD_DELETE;
  else if (streq(s, "CONNECT")) return HTTPSERV_HTTP_METHOD_CONNECT;
  else if (streq(s, "OPTIONS")) return HTTPSERV_HTTP_METHOD_OPTIONS;
  else if (streq(s, "TRACE")) return HTTPSERV_HTTP_METHOD_TRACE;
  else if (streq(s, "PATCH")) return HTTPSERV_HTTP_METHOD_PATCH;
  else return HTTPSERV_HTTP_METHOD_INVALID;
}

httpserv_http_request_t *
httpserv_http_request_new(const httpserv_http_method_t method,
                          const char *path) {
  httpserv_http_request_t *req = malloc(sizeof(httpserv_http_request_t));
  if (!req)
    return NULL;
  req->method = method;
  req->path = malloc(strlen(path) + 1);
  strcpy(req->path, path);
  req->headers = NULL;
  req->numheaders = 0;
  req->body = NULL;
  req->content_length = 0;
  return req;
}
httpserv_http_request_t *httpserv_http_request_from_raw(httpserv_raw_data_t *raw) {
  size_t pos = 0;
  char *smethod = malloc(1);
  smethod[0] = '\0';
  for (int i=pos; i<raw->len; i++) {
    pos++;
    if (raw->content[i] == ' ') break;
    smethod = realloc(smethod, strlen(smethod)+2);
    strncat(smethod, &raw->content[i], 1);
  }
  httpserv_http_method_t method = httpserv_http_strtomethod(smethod);
  if (method == HTTPSERV_HTTP_METHOD_INVALID) {
    free(smethod);
    return NULL;
  }
  char *path = malloc(1);
  path[0] = '\0';
  for (int i=pos; i<raw->len; i++) {
    pos++;
    if (i >= raw->len-1) {
      free(smethod);
      free(path);
      return NULL;
    }
    if (raw->content[i] == ' ') {
      pos++;
      break;
    }
    path = realloc(path, strlen(path)+2);
    strncat(path, &raw->content[i], 1);
  }
  for (int i=pos; i<raw->len; i++) {
    pos++;
    if (raw->content[i] == '\r' && raw->content[i+1] == '\n') {
      pos++;
      break;
    }
  }
  httpserv_http_request_t *req = httpserv_http_request_new(method, path);
  char *key = malloc(1);
  char *value = malloc(1);
  key[0] = '\0';
  value[0] = '\0';
  int keyread = 0;
  for (int i=pos; i<raw->len; i++) {
    pos++;
    if (i >= raw->len-1) {
      free(key);
      free(value);
      free(path);
      httpserv_http_request_destroy(req);
      return NULL;
    }
    // if there are no headers at all
    if (streq(key, "") && streq(value, "") && raw->content[i] == '\r' && raw->content[i+1] == '\n') {
      break;
    }

    if (!keyread && raw->content[i] == ':' && raw->content[i+1] == ' ') {
      pos++;
      i++;
      keyread = 1;
      continue;
    } else if (!keyread) {
      key = realloc(key, strlen(key)+2);
      strncat(key, &raw->content[i], 1);
    } else if (keyread && raw->content[i] == '\r' && raw->content[i+1] == '\n') {
      pos++;
      i++;
      keyread = 0;
      httpserv_http_request_add_header(req, key, value);
      memset(key, '\0', strlen(key)+1);
      memset(value, '\0', strlen(value)+1);
    } else {
      value = realloc(value, strlen(value)+2);
      strncat(value, &raw->content[i], 1);
    }
  }
  free(smethod);
  free(path);
  free(key);
  free(value);
  return req;
}
void httpserv_http_request_set_body(httpserv_http_request_t *req,
                                    const char *body, size_t len) {
  req->body = realloc(req->body, len);
  memcpy(req->body, body, len);
  req->content_length = len;
}
void httpserv_http_request_add_header(httpserv_http_request_t *req,
                                      const char *key, const char *value) {
  req->numheaders++;
  req->headers = realloc(req->headers, req->numheaders * sizeof(char *));
  req->headers[req->numheaders - 1] = malloc(strlen(key) + strlen(value) + 3);
  snprintf(req->headers[req->numheaders - 1], strlen(key) + strlen(value) + 3,
           "%s: %s", key, value);
}
void httpserv_http_request_destroy(httpserv_http_request_t *req) {
  free(req->path);
  if (req->headers) {
    for (int i = 0; i < req->numheaders; i++)
      free(req->headers[i]);
    free(req->headers);
  }
  if (req->body)
    free(req->body);
}
char *httpserv_http_request_get_header(const httpserv_http_request_t *req, const char *key) {
  if (!req) return NULL;
  char *header = NULL;
  for (int i=0; i<req->numheaders; i++) {
    if (strncmp(req->headers[i], key, strlen(key)) == 0) {
      header = req->headers[i];
      header += strlen(key)+2;
      break;
    }
  }
  return header;
}
httpserv_raw_data_t *
httpserv_http_request_build(const httpserv_http_request_t *req) {
  // 9 for 'HTTP/1.1 ', 1 for the space after the method, 4 for '\r\n\r\n'
  size_t len = 9 + strlen(httpserv_http_strmethod(req->method)) + 1 +
               strlen(req->path) + 4 + req->content_length;
  for (int i = 0; i < req->numheaders; i++) {
    len += strlen(req->headers[i]) + 2;
  }
  char *content = malloc(len);
  httpserv_raw_data_t *raw =
      malloc(sizeof(httpserv_raw_data_t));
  // TODO: build the request
  raw->content = content;
  raw->len = len;
  return raw;
}
