#include "net/connector.h"
#include "net/ssl.h"
#include <stdlib.h>
#include <stdio.h>
#include <openssl/ssl.h>
#include <unistd.h>

httpserv_net_connector_t *httpserv_net_connector_init() {
  httpserv_net_connector_t *connector =
      malloc(sizeof(httpserv_net_connector_t));
  return connector;
}
void httpserv_net_connector_read(httpserv_net_connector_t *conn,
                                 const httpserv_net_connector_kind_t conn_kind,
                                 void *buffer, const size_t bytes) {
  switch (conn_kind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    read(conn->sck_conn, buffer, bytes);
    break;
  case HTTPSERV_NET_CONNECTOR_SSL:
    SSL_read(conn->ssl_conn->ssl, buffer, bytes);
    break;
  }
}
void httpserv_net_connector_write(httpserv_net_connector_t *conn,
                                  const httpserv_net_connector_kind_t kind,
                                  void *buffer, const size_t bytes) {
  switch (kind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    write(conn->sck_conn, buffer, bytes);
    break;
  case HTTPSERV_NET_CONNECTOR_SSL:
    SSL_write(conn->ssl_conn->ssl, buffer, bytes);
    break;
  }
}
int httpserv_net_connector_get_fd(httpserv_net_connector_t *conn,
                                  const httpserv_net_connector_kind_t ckind) {
  switch (ckind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    return conn->sck_conn;
  case HTTPSERV_NET_CONNECTOR_SSL:
    return SSL_get_fd(conn->ssl_conn->ssl);
  default:
    return -1;
  }
}
void httpserv_net_connector_destroy(httpserv_net_connector_t *conn,
                                    const httpserv_net_connector_kind_t kind) {
  switch (kind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    close(conn->sck_conn);
    break;
  case HTTPSERV_NET_CONNECTOR_SSL:
    httpserv_ssl_destroy(conn->ssl_conn);
    break;
  }
  free(conn);
}
void httpserv_net_connector_destroy_connection(
    httpserv_net_connector_t *conn, const httpserv_net_connector_kind_t ckind) {
  switch (ckind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    close(conn->sck_conn);
    break;
  case HTTPSERV_NET_CONNECTOR_SSL:
    int fd = SSL_get_fd(conn->ssl_conn->ssl);
    close(fd);
    SSL_free(conn->ssl_conn->ssl);
    break;
  }
  free(conn);
}
