#ifndef __HTTPSERV_HTTP_RESPONSE_H__
#define __HTTPSERV_HTTP_RESPONSE_H__
#include <stdlib.h>
#include "http/status.h"
#include "data.h"

typedef struct {
  httpserv_http_status_t status;
  char **headers;
  char *body;
  size_t numheads;
  size_t bodylength;
  size_t lenght;
} httpserv_http_response_t;

/*
 * A simple struct containing the raw content of some response, that is to be
 * sent over the network and its length
 */
httpserv_http_response_t *
httpserv_http_response_new(const httpserv_http_status_t status);
void httpserv_http_response_add_header(httpserv_http_response_t *response,
                                       const char *key, const char *value);
/*
 * This function sets the associated body for response.
 * RETURNS:
 * - 0: Success
 * - 1: Allocation of memory failed
 * - -1: response is NULL
 */
int httpserv_http_response_set_body(httpserv_http_response_t *response,
                                    const char *body, size_t lenght);
/*
 * The returned object must be deallocated by calling free(). Additionally the
 * content member must be deallocated aswell
 */

httpserv_raw_data_t *
httpserv_http_response_build(httpserv_http_response_t *response);

void httpserv_http_response_destroy(httpserv_http_response_t *response);
/*
 * The default page of libhttpserv
 */
char *httpserv_http_response_default();
/*
 * Produces a server version formatted for the Server header. String needs to be
 * deallocated after use
 */
char *httpserv_http_server_version();
#endif
