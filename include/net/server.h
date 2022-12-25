#ifndef __HTTPSERV_HTTPSERVER_H__
#define __HTTPSERV_HTTPSERVER_H__
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "threadpool.h"
typedef struct {
  int socket;
  struct sockaddr_in *addr;
  threadpool_t *tp;
  int alive;
  int keep_alive;
  char *ip;
  in_port_t port;
} httpserv_httpserver_t;

/*
 * This function creates a new server and already creates the socket used for
 * the connection and binds said socket
 */
httpserv_httpserver_t *httpserv_httpserver_new(const char *ipaddr,
                                               const uint16_t port);
/*
 * Run a server. This function returns 0 if the server ran successful
 */
int httpserv_httpserver_run(httpserv_httpserver_t *server, size_t threads);
void httpserv_httpserver_destroy(httpserv_httpserver_t *server);
#endif
