#include "net/server.h"
#include "http/status.h"
#include "logging.h"
#include "net/connector.h"
#include "net/ssl.h"
#include "threadpool.h"
#include "http/response.h"
#include "http/request.h"
#include "data.h"
#include "router.h"
#include "resource.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
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
  httpserv_net_connector_t *conn;
  httpserv_net_connector_kind_t ckind;
  httpserv_router_t *router;
  httpserv_resource_loader_t *resload;
};

// worker function for the httpserver
void *httpserv_httpserver_worker(void *arg) {
  struct HttpservWorkerData *data = (struct HttpservWorkerData *)arg;
  if (!data->conn) {
    free(arg);
    return NULL;
  }
  if (data->ckind == HTTPSERV_NET_CONNECTOR_SSL) {
    if (SSL_accept(data->conn->ssl_conn->ssl) == -1) {
      httpserv_logging_err(
          "SSL handshake failed: %s",
          ERR_error_string(SSL_get_error(data->conn->ssl_conn->ssl, -1), NULL));
      free(data);
      return NULL;
    }
  }
  httpserv_raw_data_t *rawreq = httpserv_raw_data_init();
  rawreq->content = malloc(1);
  rawreq->content[0] = '\0';
  char c = '\0';
  while (1) {
    httpserv_net_connector_read(data->conn, data->ckind, &c, 1);
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
        httpserv_net_connector_destroy_connection(data->conn, data->ckind);
        return NULL;
      }
      char *content = malloc(clen);
      httpserv_net_connector_read(data->conn, data->ckind, content, clen);
      httpserv_http_request_set_body(req, content, clen);
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
    char *path = NULL;
    httpserv_route_t *route = httpserv_router_lookup(data->router, path);
    if (route) {
      switch (route->kind) {
      case HTTPSERV_ROUTE_KIND_ALIAS:
        path = route->routepattern;
        break;
      case HTTPSERV_ROUTE_KIND_REDIRECT:
        resp->status = HTTPSERV_HTTP_STATUS_MOVED_PERMANENTLY;
        break;
      case HTTPSERV_ROUTE_KIND_ROUTE:
        resp->status = HTTPSERV_HTTP_STATUS_FOUND;
        break;
      }
    } else {
      path = req->path;
    }
    httpserv_resource_t *res = NULL;
    if ((route && route->kind == HTTPSERV_ROUTE_KIND_ALIAS) || path) {
      res = httpserv_resource_loader_load(data->resload, path);
      if (!res) {
        resp->status = HTTPSERV_HTTP_STATUS_NOT_FOUND;
        char *body = httpserv_http_response_default();
        int body_len_size = (int)log10(strlen(body)) + 2;
        char *body_len = malloc(body_len_size);
        httpserv_http_response_set_body(resp, body, strlen(body));
        httpserv_http_response_add_header(resp, "Content-Length", body_len);
        // free(body);
        free(body_len);
      } else {
        int body_len_size = (int)log10(res->size) + 2;
        char *body_len = malloc(body_len_size);
        snprintf(body_len, body_len_size, "%zu", res->size);
        httpserv_http_response_add_header(resp, "Content-Lenght", body_len);
        free(body_len);
      }
    } else {
      httpserv_http_response_add_header(resp, "Location", route->routepattern);
    }
    char *version = httpserv_http_server_version();
    httpserv_http_response_add_header(resp, "Server", version);
    free(version);
    httpserv_raw_data_t *raw = httpserv_http_response_build(resp);
    if (!raw) {
      httpserv_logging_err("failed to parse response object");
    } else {
      // write the headers
      httpserv_net_connector_write(data->conn, data->ckind, raw->content,
                                   raw->len);
      if (res) {
        httpserv_resource_write_to_conn(res, data->conn, data->ckind);
      }
      free(raw->content);
      free(raw);
    }
  }

  httpserv_net_connector_destroy_connection(data->conn, data->ckind);
  free(data);
  return NULL;
}

httpserv_httpserver_t *
httpserv_httpserver_new(const char *ipaddr, const uint16_t port,
                        const httpserv_httpserver_addr_kind_t akind) {
  httpserv_httpserver_t *serv = malloc(sizeof(httpserv_httpserver_t));
  serv->addr = malloc(sizeof(httpserv_httpserver_addr_t));
  serv->ip = malloc(strlen(ipaddr) + 1);
  strcpy(serv->ip, ipaddr);
  serv->port = port;
  serv->akind = akind;
  serv->conn = httpserv_net_connector_init();
  serv->ckind = HTTPSERV_NET_CONNECTOR_SCK;
  serv->resload = NULL;
  serv->router = NULL;
  switch (akind) {
  case HTTPSERV_HTTPSERVER_ADDR_KIND_IPv4:
    serv->addr->v4.sin_family = AF_INET;
    serv->addr->v4.sin_addr.s_addr = inet_addr(ipaddr);
    if (inet_pton(AF_INET, ipaddr, &serv->addr->v4.sin_addr) != 1) {
      httpserv_logging_err("failed to parse ip address: %s", ipaddr);
      free(serv->addr);
      free(serv->ip);
      free(serv);
      return NULL;
    }
    serv->addr->v4.sin_port = htons(port);
    serv->conn->sck_conn = socket(AF_INET, SOCK_STREAM, 0);
    break;
  case HTTPSERV_HTTPSERVER_ADDR_KIND_IPv6:
    serv->addr->v6.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, ipaddr, &serv->addr->v6.sin6_addr) != 1) {
      httpserv_logging_err("failed to parse ip address: %s", ipaddr);
      free(serv->addr);
      free(serv->ip);
      free(serv);
      return NULL;
    }
    serv->addr->v6.sin6_port = htons(port);
    serv->conn->sck_conn = socket(AF_INET6, SOCK_STREAM, 0);
    break;
  }
  serv->tp = NULL;
  serv->alive = 0;
  serv->keep_alive = 0;
  if (serv->conn->sck_conn < 0) {
    httpserv_logging_err("failed to create socket: %s", strerror(errno));
    free(serv->addr);
    free(serv->ip);
    free(serv);
    return NULL;
  }
  if (bind(serv->conn->sck_conn, (struct sockaddr *)serv->addr,
           sizeof(*serv->addr)) < 0) {
    httpserv_logging_err("failed to bind socket: %s", strerror(errno));
    httpserv_httpserver_destroy(serv);
    return NULL;
  }
  return serv;
}

void httpserv_httpserver_use_ssl(httpserv_httpserver_t *srv,
                                 const char *privkeyfile,
                                 const char *certfile) {
  if (!srv)
    return;
  srv->ckind = HTTPSERV_NET_CONNECTOR_SSL;
  int fd = srv->conn->sck_conn;
  srv->conn->ssl_conn = httpserv_ssl_init(privkeyfile, certfile);
  SSL_set_fd(srv->conn->ssl_conn->ssl, fd);
}
void httpserv_httpserver_add_router(httpserv_httpserver_t *srv,
                                    httpserv_router_t *router) {
  if (!srv)
    return;
  srv->router = router;
}
void httpserv_httpserver_add_resource_loader(
    httpserv_httpserver_t *srv, httpserv_resource_loader_t *resload) {
  if (!srv)
    return;
  srv->resload = resload;
}

/*
 * returns 0 if the server ran successfully and -1 if an error was encountered.
 * Even though an error was encountered you still have to destroy the server
 * manually
 */
int httpserv_httpserver_run(httpserv_httpserver_t *server, size_t threads) {
  httpserv_logging_log("starting server...");
  server->tp = threadpool_new(threads);
  int srvfd = httpserv_net_connector_get_fd(server->conn, server->ckind);
  if (listen(srvfd, 0) == -1) {
    httpserv_logging_err("failed to listen on socket: %s", strerror(errno));
    return -1;
  } else {
    httpserv_logging_log("server listening on %s:%d", server->ip, server->port);
  }
  server->keep_alive = 1;
  server->alive = 1;
  while (server->keep_alive) {
    // TODO: use select(3) to handle sockets "asynchronously"
    int connfd = accept(srvfd, NULL, NULL);
    struct HttpservWorkerData *data = malloc(sizeof(struct HttpservWorkerData));
    httpserv_net_connector_t *conn = httpserv_net_connector_init();
    switch (server->ckind) {
    case HTTPSERV_NET_CONNECTOR_SCK:
      conn->sck_conn = connfd;
      break;
    case HTTPSERV_NET_CONNECTOR_SSL:
      conn->ssl_conn =
          httpserv_ssl_init_conn(server->conn->ssl_conn->ssl_ctx, connfd);
      break;
    }
    data->conn = conn;
    data->ckind = server->ckind;
    data->router = server->router;
    data->resload = server->resload;
    if (server->keep_alive)
      threadpool_add_work(server->tp, httpserv_httpserver_worker, data);
    else
      httpserv_net_connector_destroy_connection(conn, server->ckind);
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
  httpserv_net_connector_destroy(server->conn, server->ckind);
  if (server->tp)
    threadpool_destroy(server->tp);
  free(server->addr);
  free(server->ip);
  free(server);
}
