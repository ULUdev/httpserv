#include "httpserv.h"
#include "http/response.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "http/status.h"

httpserv_http_response_t *
httpserv_http_response_new(const httpserv_http_status_t status) {
  httpserv_http_response_t *response = malloc(sizeof(httpserv_http_response_t));
  response->status = status;
  response->body = NULL;
  response->bodylength = 0;
  // 9 for 'HTTP/1.1 ', 4 for 'XXX ' and another 4 for '\r\n\r\n'
  response->lenght = 9 + 4 + strlen(httpserv_http_strstatus(status)) + 4;
  response->headers = NULL;
  response->numheads = 0;
  return response;
}
void httpserv_http_response_add_header(httpserv_http_response_t *response,
                                       const char *key, const char *value) {
  // do nothing if some value is missing
  if (!key || !response || !value)
    return;
  size_t headlen = strlen(key) + strlen(value) + 3;
  char *header = malloc(headlen);
  snprintf(header, headlen, "%s: %s", key, value);
  response->numheads++;
  response->headers = realloc(response->headers, response->numheads);
  response->headers[response->numheads - 1] = header;
  response->lenght += strlen(key) + strlen(value) + 4;
}
int httpserv_http_response_set_body(httpserv_http_response_t *response,
                                    const char *body, size_t lenght) {
  if (!response)
    return -1;
  response->body = realloc(response->body, lenght);
  if (!response->body)
    return 1;
  memcpy(response->body, body, lenght);
  response->lenght -= response->bodylength;
  response->bodylength = lenght;
  response->lenght += response->bodylength;
  return 0;
}
httpserv_http_response_raw_t *
httpserv_http_response_build(httpserv_http_response_t *response) {
  if (!httpserv_http_status_valid(response->status))
    return NULL;
  httpserv_http_response_raw_t *rawresp =
      malloc(sizeof(httpserv_http_response_raw_t));
  char status[4];
  snprintf(status, 4, "%d ", response->status);
  rawresp->content = malloc(response->lenght);
  memset(rawresp->content, '\0', response->lenght);
  strncat(rawresp->content, "HTTP/1.1 ", 10);
  strncat(rawresp->content, status, 5);
  strncat(rawresp->content, " ", 2);
  /*
   * NOTE: this is alright as in the current implementation we already know,
   * that response->status is valid
   */
  strcat(rawresp->content, httpserv_http_strstatus(response->status));
  strncat(rawresp->content, "\r\n", 3);
  if (response->numheads != 0) {
    char *buf = NULL;
    for (int i = 0; i < response->numheads; i++) {
      buf = realloc(buf, strlen(response->headers[i]) + 3);
      snprintf(buf, strlen(response->headers[i]) + 3, "%s\r\n",
               response->headers[i]);
      strcat(rawresp->content, buf);
    }
    free(buf);
  }
  strncat(rawresp->content, "\r\n", 3);
  // TODO: You can't really use strncat as it will search for null-termination
  // which might not exist
  strncat(rawresp->content, response->body, response->bodylength);
  rawresp->length = response->lenght;
  return rawresp;
}

void httpserv_http_response_destroy(httpserv_http_response_t *response) {
  for (int i = 0; i < response->numheads; i++)
    free(response->headers[i]);
  free(response->body);
  free(response->headers);
  free(response);
}
char *httpserv_http_response_default() { return "<h1>libhttpserv.so</h1>"; }
char *httpserv_http_server_version() {
  char *v = malloc(30);
  memset(v, 0, 30);
  sprintf(v, "cweb/%d.%d.%d", httpserv_version_major(),
          httpserv_version_minor(), httpserv_version_patch());
  return v;
}
