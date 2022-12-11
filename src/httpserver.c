#include "httpserver.h"
#include "http/status.h"
#include "logging.h"
#include "threadpool.h"
#include "http/response.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

struct HttpservWorkerData {
  int sockfd;
};

// worker function for the httpserver
void *httpserv_httpserver_worker(void *arg) {
  struct HttpservWorkerData *data = (struct HttpservWorkerData *)arg;
  if (data->sockfd <= 0) {
    free(arg);
    return NULL;
  }
  /*
   * Parse request here and implement actually looking up resources and routes
   * etc.
   */
  httpserv_http_response_t *resp =
      httpserv_http_response_new(HTTPSERV_HTTP_STATUS_OK);
  if (!resp) {
    httpserv_logging_err("failed to generate response object");
  } else {
    char *body = httpserv_http_response_default();
    char *version = httpserv_http_server_version();
    httpserv_http_response_set_body(resp, body, strlen(body));
    int body_len_size = (int)log10(resp->bodylength) + 2;
    char *body_len = malloc(body_len_size);
    snprintf(body_len, body_len_size, "%zu", resp->bodylength);
    httpserv_http_response_add_header(resp, "Server", version);
    httpserv_http_response_add_header(resp, "Content-Lenght", body_len);
    free(version);
    free(body_len);
    httpserv_http_response_raw_t *raw = httpserv_http_response_build(resp);
    if (!raw) {
      httpserv_logging_err("failed to parse response object");
    } else {
      // httpserv_logging_log("response: %s %d", raw->content, raw->length);
      write(data->sockfd, raw->content, raw->length);
      free(raw->content);
      free(raw);
    }
  }

  shutdown(data->sockfd, SHUT_RDWR);
  close(data->sockfd);
  free(data);
  return NULL;
}

httpserv_httpserver_t *httpserv_httpserver_new(const char *ipaddr,
                                               const uint16_t port) {
  httpserv_httpserver_t *serv = malloc(sizeof(httpserv_httpserver_t));
  serv->addr = malloc(sizeof(struct sockaddr_in));
  serv->ip = malloc(strlen(ipaddr) + 1);
  strcpy(serv->ip, ipaddr);
  serv->port = port;
  serv->addr->sin_family = AF_INET;
  serv->addr->sin_addr.s_addr = inet_addr(ipaddr);
  if (serv->addr->sin_addr.s_addr == -1) {
    httpserv_logging_err("failed to parse ip address: %s", ipaddr);
    free(serv->addr);
    free(serv->ip);
    free(serv);
    return NULL;
  }
  serv->addr->sin_port = htons(port);
  serv->socket = socket(AF_INET, SOCK_STREAM, 0);
  serv->tp = NULL;
  serv->alive = 0;
  serv->keep_alive = 0;
  if (serv->socket < 0) {
    httpserv_logging_err("failed to create socket: %s", strerror(errno));
    free(serv->addr);
    free(serv->ip);
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

/*
 * returns 0 if the server ran successfully and -1 if an error was encountered.
 * Even though an error was encountered you still have to destroy the server
 * manually
 */
int httpserv_httpserver_run(httpserv_httpserver_t *server, size_t threads) {
  httpserv_logging_log("starting server...");
  server->tp = threadpool_new(threads);
  if (listen(server->socket, 0) == -1) {
    httpserv_logging_err("failed to listen on socket: %s", strerror(errno));
    return -1;
  } else {
    httpserv_logging_log("server listening on %s:%d", server->ip, server->port);
  }
  server->keep_alive = 1;
  server->alive = 1;
  while (server->keep_alive) {
    // TODO: use select(3) to handle sockets "asynchronously"
    int conn = accept(server->socket, NULL, NULL);
    struct HttpservWorkerData *data = malloc(sizeof(struct HttpservWorkerData));
    data->sockfd = conn;
    threadpool_add_work(server->tp, httpserv_httpserver_worker, data);
  }
  server->alive = 0;
  return 0;
}
void httpserv_httpserver_destroy(httpserv_httpserver_t *server) {
  if (!server)
    return;
  server->keep_alive = 0;
  while (server->alive)
    ;
  if (close(server->socket) == -1)
    httpserv_logging_err("failed to close socket: %s", strerror(errno));
  if (server->tp)
    threadpool_destroy(server->tp);
  free(server->addr);
  free(server->ip);
  free(server);
}
