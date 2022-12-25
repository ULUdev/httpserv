#ifndef __HTTPSERV_NET_CONNECTOR_H__
#define __HTTPSERV_NET_CONNECTOR_H__
#include "net/ssl.h"
#include <stddef.h>

typedef enum {
  HTTPSERV_NET_CONNECTOR_SSL,
  HTTPSERV_NET_CONNECTOR_SCK,
} httpserv_net_connector_kind_t;

typedef union {
  httpserv_ssl_t *ssl_conn;
  int sck_conn;
} httpserv_net_connector_t;

httpserv_net_connector_t *httpserv_net_connector_init();
void httpserv_net_connector_read(httpserv_net_connector_t *conn,
                                 const httpserv_net_connector_kind_t conn_kind,
                                 void *buffer, const size_t bytes);
void httpserv_net_connector_write(httpserv_net_connector_t *conn,
                                  const httpserv_net_connector_kind_t kind,
                                  void *buffer, const size_t bytes);
void httpserv_net_connector_destroy(httpserv_net_connector_t *conn,
                                    const httpserv_net_connector_kind_t kind);

#endif
