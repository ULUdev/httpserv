#ifndef __HTTPSERV_HTTPSERVER_H__
#define __HTTPSERV_HTTPSERVER_H__
#include <sys/socket.h>
typedef struct {
  int socket;
  struct sockaddr_in *addr;
} httpserv_httpserver_t;

/*
 * This function creates a new server and already creates the socket used for
 * the connection and binds said socket
 */
httpserv_httpserver_t *httpserv_httpserver_new(const char *ipaddr, const int port);
// returns 0 if server ran successfully
int httpserv_httpserver_run(httpserv_httpserver_t *server);
void httpserv_httpserver_destroy(httpserv_httpserver_t *server);
#endif
