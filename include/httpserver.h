#ifndef __HTTPSERV_HTTPSERVER_H__
#define __HTTPSERV_HTTPSERVER_H__
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
  // might change type to something else
  uint16_t port;
} httpserv_httpserver_t;

/*
 * This function creates a new server and already creates the socket used for
 * the connection and binds said socket
 */
httpserv_httpserver_t *httpserv_httpserver_new(const char *ipaddr, const uint16_t port);
// returns 0 if server ran successfully
int httpserv_httpserver_run(httpserv_httpserver_t *server, size_t threads);
void httpserv_httpserver_destroy(httpserv_httpserver_t *server);
#endif
