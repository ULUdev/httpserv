#include "httpserver.h"
#include "logging.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

httpserv_httpserver_t *httpserv_httpserver_new(const char *ipaddr,
                                               const int port) {
  httpserv_httpserver_t *serv = malloc(sizeof(httpserv_httpserver_t));
  serv->addr = malloc(sizeof(struct sockaddr_in));
  serv->addr->sin_family = AF_INET;
  serv->addr->sin_addr.s_addr = inet_addr(ipaddr);
  serv->socket = socket(AF_INET, SOCK_STREAM, 0);
  if (serv->socket < 0) {
    httpserv_logging_err("failed to create socket: %s", strerror(errno));
    free(serv->addr);
    free(serv);
    return NULL;
  }
  if (bind(serv->socket, (struct sockaddr *)serv->addr, sizeof(*serv->addr)) <
      0) {
    httpserv_logging_err("failed to bind socket: %s", strerror(errno));
    httpserv_httpserver_destroy(serv);
    return NULL;
  }
  return serv;
}
// returns 0 if server ran successfully
int httpserv_httpserver_run(httpserv_httpserver_t *server);
void httpserv_httpserver_destroy(httpserv_httpserver_t *server) {
  if (close(server->socket))
    httpserv_logging_err("failed to shutdown socket: %s", strerror(errno));
  free(server->addr);
  free(server);
}
