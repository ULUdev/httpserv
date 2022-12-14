#include "data.h"
#include <stdlib.h>

httpserv_raw_data_t *httpserv_raw_data_init() {
  httpserv_raw_data_t *data = malloc(sizeof(httpserv_raw_data_t));
  data->content = NULL;
  data->len = 0;
  return data;
}
void httpserv_raw_data_destroy(httpserv_raw_data_t *data);
