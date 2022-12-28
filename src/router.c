#include "router.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pthread.h>
#include "logging.h"

httpserv_router_t *httpserv_router_new() {
  httpserv_router_t *router = malloc(sizeof(httpserv_router_t));
  router->mutex = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(router->mutex, NULL) != 0) {
    free(router->mutex);
    free(router);
    return NULL;
  }
  router->head = NULL;
  return router;
}
void httpserv_router_add_route(httpserv_router_t *router,
                               httpserv_route_t *route) {
  if (!router)
    return;
  pthread_mutex_lock(router->mutex);
  route->next = router->head;
  router->head = route;
  pthread_mutex_unlock(router->mutex);
}
httpserv_route_t *httpserv_router_lookup(httpserv_router_t *router,
                                         const char *query) {
  if (!router || !query)
    return NULL;
  pthread_mutex_lock(router->mutex);
  httpserv_route_t *cur = router->head;
  while (cur) {
    regex_t re;
    if (regcomp(&re, cur->pattern, 0) != 0)
      return NULL;
    if (regexec(&re, query, 0, NULL, 0) == 0)
      break;
    cur = cur->next;
  }
  pthread_mutex_unlock(router->mutex);
  return cur;
}
void httpserv_router_destroy(httpserv_router_t *router) {
  httpserv_route_t *cur = router->head;
  pthread_mutex_destroy(router->mutex);
  while (cur) {
    httpserv_route_t *next = cur->next;
    httpserv_route_destroy(cur);
    cur = next;
  }
  free(router->mutex);
  free(router);
}

httpserv_route_t *httpserv_route_new(const char *pattern,
                                     const char *routepattern,
                                     const httpserv_route_kind_t kind) {
  httpserv_route_t *route = malloc(sizeof(httpserv_route_t));
  route->kind = kind;
  route->next = NULL;
  route->pattern = malloc(strlen(pattern) + 1);
  route->routepattern = malloc(strlen(routepattern) + 1);
  strcpy(route->pattern, pattern);
  strcpy(route->routepattern, routepattern);
  return route;
}
void httpserv_route_destroy(httpserv_route_t *route) {
  free(route->pattern);
  free(route->routepattern);
  free(route);
}
