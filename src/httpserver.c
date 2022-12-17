#include "httpserver.h"
#include "http/status.h"
#include "logging.h"
#include "threadpool.h"
#include "http/response.h"
#include "http/request.h"
#include "data.h"
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
#include <errno.h>

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
  httpserv_raw_data_t *rawreq = httpserv_raw_data_init();
  rawreq->content = malloc(1);
  rawreq->content[0] = '\0';
  char c = '\0';
  while (1) {
    read(data->sockfd, &c, 1);
    rawreq->content = realloc(rawreq->content, rawreq->len + 2);
    strncat(rawreq->content, &c, 1);
    if (rawreq->len >= 4 && rawreq->content[rawreq->len] == '\n' &&
        rawreq->content[rawreq->len - 1] == '\r' &&
        rawreq->content[rawreq->len - 2] == '\n' &&
        rawreq->content[rawreq->len - 3] == '\r') {
      rawreq->len++;
      break;
    }
    rawreq->len++;
  }
  httpserv_http_request_t *req = httpserv_http_request_from_raw(rawreq);
  if (req) {
    char *header = httpserv_http_request_get_header(req, "Content-Length");
    if (header) {
      size_t clen = (size_t)strtol(header, NULL, 10);
      if (!clen) {
        httpserv_logging_err("failed to convert number: %s", strerror(errno));
        shutdown(data->sockfd, SHUT_RDWR);
        close(data->sockfd);
        return NULL;
      }
      char *content = malloc(clen);
      if (read(data->sockfd, content, clen) > 0) {
        httpserv_http_request_set_body(req, content, clen);
      }
    }
  }
  httpserv_http_status_t status = HTTPSERV_HTTP_STATUS_OK;
  if (!req) {
    status = HTTPSERV_HTTP_STATUS_BAD_REQUEST;
    httpserv_logging_log("bad request");
  } else {
    httpserv_logging_log("request: %s %s", httpserv_http_strmethod(req->method),
                         req->path);
  }
  httpserv_http_response_t *resp = httpserv_http_response_new(status);
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
    httpserv_raw_data_t *raw = httpserv_http_response_build(resp);
    if (!raw) {
      httpserv_logging_err("failed to parse response object");
    } else {
      // httpserv_logging_log("response: %s %d", raw->content, raw->length);
      write(data->sockfd, raw->content, raw->len);
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
