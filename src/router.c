#include "router.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pthread.h>
#include "logging.h"
#include "stringh.h"

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
    if (regexec(&cur->pattern, query, 0, NULL, 0) == 0)
      break;
    cur = cur->next;
  }
  if (cur) {
    if (httpserv_streq(cur->routepattern, query) == 0) {
      pthread_mutex_unlock(router->mutex);
      return NULL;
    }
    switch (cur->kind) {
    case HTTPSERV_ROUTE_KIND_ALIAS:
      httpserv_logging_log("alias: %s => %s", query, cur->routepattern);
      break;
    case HTTPSERV_ROUTE_KIND_REDIRECT:
      httpserv_logging_log("redirecting (301): %s => %s", query, cur->routepattern);
      break;
    case HTTPSERV_ROUTE_KIND_ROUTE:
      httpserv_logging_log("routing (302): %s => %s", query, cur->routepattern);
      break;
    }
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
  if (regcomp(&route->pattern, pattern, 0) != 0) {
    httpserv_logging_err("failed to compile regex");
    free(route);
    return NULL;
  }
  route->routepattern = malloc(strlen(routepattern) + 1);
  strcpy(route->routepattern, routepattern);
  return route;
}
void httpserv_route_destroy(httpserv_route_t *route) {
  regfree(&route->pattern);
  free(route->routepattern);
  free(route);
}

// functions to parse a file

// returns NULL if parsing failed and it will print an error message to stderr
httpserv_route_t *__httpserv_parse_line(const char *line) {
  char *pattern = malloc(1);
  char *routepattern = malloc(1);
  httpserv_route_kind_t kind = HTTPSERV_ROUTE_KIND_ALIAS;
  strcpy(pattern, "\0");
  strcpy(routepattern, "\0");
  int pattern_read = 0;
  for (int i=0; i<strlen(line); i++) {
    if (pattern_read) {
      if (line[i] == '|') {
	if (strncmp(line+i, "route", 0) == 0) {
	  kind = HTTPSERV_ROUTE_KIND_ROUTE;
	} else if (strncmp(line+i, "alias", 0) == 0) {
	  kind = HTTPSERV_ROUTE_KIND_ALIAS;
	} else if (strncmp(line+i, "redirect", 0) == 0) {
	  kind = HTTPSERV_ROUTE_KIND_REDIRECT;
	} else {
	  fprintf(stderr, "error parsing routes: found '|' but no valid expression afterwards\n");
	}
	break;
      }
      routepattern = realloc(routepattern, strlen(routepattern) + 2);
      strncat(routepattern, &line[i], 1);
      continue;
    }
    if (strncmp(line+i, "=>", 2) == 0) {
      pattern_read = 1;
      i++;
      continue;
    }
    pattern = realloc(pattern, strlen(pattern) +2);
    strncat(pattern, &line[i], 1);
  }
  httpserv_route_t *route = httpserv_route_new(pattern, routepattern, kind);
  free(pattern);
  free(routepattern);
  return route;
}

httpserv_router_t *httpserv_router_parse_from_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    perror("failed to open file");
    return NULL;
  }
  char *line;
  httpserv_router_t *router = httpserv_router_new();
  while (fscanf(f, "%ms\n", &line) == 1) {
    httpserv_route_t *route = __httpserv_parse_line(line);
    if (route) httpserv_router_add_route(router, route);
    else {
      httpserv_router_destroy(router);
      break;
    }
  }
  fclose(f);
  free(line);
  return router;
}
