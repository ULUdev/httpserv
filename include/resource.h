#ifndef __HTTPSERV_RESOURCE_H__
#define __HTTPSERV_RESOURCE_H__
#include "net/connector.h"
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>

typedef struct HttpservResource {
  char *path;
  void *contents;
  size_t size;
  int fd;
  struct HttpservResource *next;
} httpserv_resource_t;

typedef struct {
  httpserv_resource_t *head;
  pthread_mutex_t *mutex;
  char *root;
} httpserv_resource_loader_t;

httpserv_resource_t *httpserv_resource_new(const char *path);
void httpserv_resource_write_to_fd(httpserv_resource_t *res, int fd);
void httpserv_resource_write_to_conn(httpserv_resource_t *res,
                                     httpserv_net_connector_t *conn,
                                     const httpserv_net_connector_kind_t ckind);
/*
 * Load the contents of the accociated file into res
 */
void httpserv_resource_destroy(httpserv_resource_t *res);

httpserv_resource_loader_t *httpserv_resource_loader_new(const char *root);
void httpserv_resource_loader_preload(httpserv_resource_loader_t *resload,
                                      const char *path);
/*
 * load a resource. If the resource hasn't already been loaded load it
 * aswell. If anything goes wrong while loading NULL is returend.
 */
httpserv_resource_t *
httpserv_resource_loader_load(httpserv_resource_loader_t *resload,
                              const char *path);
void httpserv_resource_loader_destroy(httpserv_resource_loader_t *resload);

#endif
