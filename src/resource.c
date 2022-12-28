#include "resource.h"
#include "logging.h"
#include "net/connector.h"
#include "stringh.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <openssl/ssl.h>

httpserv_resource_t *httpserv_resource_new(const char *path) {
  if (access(path, F_OK | R_OK) == -1)
    return NULL;
  struct stat fstats;
  if (stat(path, &fstats) != 0)
    return NULL;
  size_t fsize = fstats.st_size;
  httpserv_resource_t *res = malloc(sizeof(httpserv_resource_t));
  res->size = fsize;
  res->path = malloc(strlen(path) + 1);
  res->fd = open(path, O_RDONLY);
  if (res->fd < 0) {
    httpserv_logging_err("failed to open file: %s", strerror(errno));
    free(res->path);
    free(res);
    return NULL;
  }
  res->contents = mmap(NULL, fsize, PROT_READ, MAP_SHARED, res->fd, 0);
  if (res->contents == MAP_FAILED) {
    httpserv_logging_err("failed to map file: %s", strerror(errno));
    free(res->path);
    free(res);
    return NULL;
  }
  res->next = NULL;
  strcpy(res->path, path);
  return res;
}
void httpserv_resource_write_to_fd(httpserv_resource_t *res, int fd) {
  if (!res)
    return;
  write(fd, res->contents, res->size);
}
void httpserv_resource_write_to_conn(
    httpserv_resource_t *res, httpserv_net_connector_t *conn,
    const httpserv_net_connector_kind_t ckind) {
  if (!res || !conn)
    return;
  switch (ckind) {
  case HTTPSERV_NET_CONNECTOR_SCK:
    write(conn->sck_conn, res->contents, res->size);
    break;
  case HTTPSERV_NET_CONNECTOR_SSL:
    SSL_write(conn->ssl_conn->ssl, res->contents, res->size);
    break;
  }
}
void httpserv_resource_destroy(httpserv_resource_t *res) {
  if (!res)
    return;
  munmap(res->contents, res->size);
  free(res->path);
  free(res);
}

httpserv_resource_loader_t *httpserv_resource_loader_new(const char *root) {
  if (!root)
    return NULL;
  httpserv_resource_loader_t *resload =
      malloc(sizeof(httpserv_resource_loader_t));
  resload->root = malloc(strlen(root) + 1);
  strcpy(resload->root, root);
  resload->mutex = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(resload->mutex, NULL) != 0) {
    free(resload->root);
    free(resload);
    return NULL;
  }
  resload->head = NULL;
  return resload;
}
void httpserv_resource_loader_preload(httpserv_resource_loader_t *resload,
                                      const char *path) {
  if (!resload || !path)
    return;
  pthread_mutex_lock(resload->mutex);
  char *name = malloc(strlen(resload->root) + strlen(path) + 1);
  strncpy(name, resload->root, strlen(resload->root));
  strcat(name, path);
  httpserv_resource_t *cur = resload->head;
  while (cur) {
    if (httpserv_streq(cur->path, name) == 0) {
      // the resource is already loaded
      free(name);
      return;
    }
    cur = cur->next;
  }
  httpserv_resource_t *res = httpserv_resource_new(name);
  res->next = resload->head;
  resload->head = res;
  pthread_mutex_unlock(resload->mutex);
  free(name);
}
httpserv_resource_t *
httpserv_resource_loader_load(httpserv_resource_loader_t *resload,
                              const char *path) {
  if (!resload || !path)
    return NULL;
  pthread_mutex_lock(resload->mutex);
  char *real_path = malloc(strlen(resload->root) + strlen(path) + 1);
  strncpy(real_path, resload->root, strlen(resload->root) + 1);
  strcat(real_path, path);
  httpserv_resource_t *cur = resload->head;
  while (cur) {
    if (httpserv_streq(cur->path, real_path) == 0)
      break;
    cur = cur->next;
  }
  if (!cur) {
    cur = httpserv_resource_new(real_path);
  }
  free(real_path);
  pthread_mutex_unlock(resload->mutex);
  return cur;
}
void httpserv_resource_loader_destroy(httpserv_resource_loader_t *resload) {
  if (!resload)
    return;
  pthread_mutex_destroy(resload->mutex);
  httpserv_resource_t *cur = resload->head;
  while (cur) {
    httpserv_resource_t *next = cur->next;
    free(cur);
    cur = next;
  }
  free(resload->mutex);
  free(resload->root);
  free(resload);
}
