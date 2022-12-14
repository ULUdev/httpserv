#ifndef __HTTPSERV_DATA_H__
#define __HTTPSERV_DATA_H__
#include <stdlib.h>
typedef char httpserv_byte_t;

/*
 * A type for data that is not necessarily a string (may contain NULs in places
 * other than the end)
 */
typedef struct {
  httpserv_byte_t *content;
  size_t len;
} httpserv_raw_data_t;
httpserv_raw_data_t *httpserv_raw_data_init();
void httpserv_raw_data_destroy(httpserv_raw_data_t *data);
#endif
