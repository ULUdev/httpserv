#ifndef __HTTPSERV_ROUTER_H__
#define __HTTPSERV_ROUTER_H__
#include <bits/pthreadtypes.h>
#include <pthread.h>

/*
 * An enum defining the different kinds of routes supported
 *
 * # Options
 * - `HTTPSERV_ROUTE_KIND_ROUTE`: a route will be handled with status code 302
 * - `HTTPSERV_ROUTE_KIND_ALIAS`: a route will be handled as an alias (like `/`
     on most websites just respondes with the contents of `/index.html`)
 * - `HTTPSERV_ROUTE_KIND_REDIRECT`: a route is a redirect handled with status
 code 301
 */
typedef enum {
  HTTPSERV_ROUTE_KIND_ROUTE,
  HTTPSERV_ROUTE_KIND_ALIAS,
  HTTPSERV_ROUTE_KIND_REDIRECT,
} httpserv_route_kind_t;

typedef struct HttpservRoute {
  char *pattern;
  char *routepattern;
  httpserv_route_kind_t kind;
  struct HttpservRoute *next;
} httpserv_route_t;

typedef struct {
  httpserv_route_t *head;
  pthread_mutex_t *mutex;
} httpserv_router_t;

httpserv_router_t *httpserv_router_new();
void httpserv_router_add_route(httpserv_router_t *router,
                               httpserv_route_t *route);
httpserv_route_t *httpserv_router_lookup(httpserv_router_t *router,
                                         const char *query);
void httpserv_router_destroy(httpserv_router_t *router);

httpserv_route_t *httpserv_route_new(const char *pattern,
                                     const char *routepattern,
                                     const httpserv_route_kind_t kind);
void httpserv_route_destroy(httpserv_route_t *route);
#endif
