#include "config.h"
#include "logging.h"
#include "node.h"
#include "threadpool.h"
#include "stringh.h"
#include "tree.h"
#include "httpserver.h"
#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// macros
#define streq(s1, s2) (httpserv_streq(s1, s2) == 0)
#define CWEB_VERSION_MAJOR 0
#define CWEB_VERSION_MINOR 1
#define CWEB_VERSION_PATCH 0
#define CWEB_THREADS_DEFAULT 10


// globals
tree_t *cfg = NULL;
httpserv_httpserver_t *srv = NULL;

void cweb_cleanup() {
  if (cfg) httpserv_config_destroy(cfg);
  if (srv) httpserv_httpserver_destroy(srv);
  httpserv_logging_destroy();
}

void handle_sigint(int signal) {
  write(STDOUT_FILENO, "got SIGINT. Stopping server...\n", 11);
  if (srv->alive == 0) {
    cweb_cleanup();
    exit(EXIT_SUCCESS);
  }
  srv->keep_alive = 0;
}

const char *CWEB_HELP_STR =
    "\n"
    "OPTIONS\n"
    "  -h,--help: print this help and exit\n"
    "  -v,--version: print the version number and exit\n"
    "  -f,--file <file>: use <file> instead of the default configuration "
    "file\n";

int main(int argc, char **argv) {
  // signal handler
  // struct sigaction act;
  // act.sa_handler = handle_sigint;

  // consider using sigaction instead
  signal(SIGINT, &handle_sigint);

  char *path = "/etc/cweb/cweb.conf";
  int threads = CWEB_THREADS_DEFAULT;
  int return_value = EXIT_SUCCESS;
  for (int i = 0; i < argc; i++) {
    if (streq(argv[i], "-h") || streq(argv[i], "--help")) {
      fprintf(stderr, "%s\n", CWEB_HELP_STR);
      exit(EXIT_SUCCESS);
    } else if (streq(argv[i], "-v") || streq(argv[i], "--version")) {
      fprintf(stderr, "cweb version: %d.%d.%d\n", CWEB_VERSION_MAJOR,
              CWEB_VERSION_MINOR, CWEB_VERSION_PATCH);
      exit(EXIT_SUCCESS);
    } else if (streq(argv[i], "-f") || streq(argv[i], "--file")) {
      if (i == argc - 1) {
        fprintf(stderr, "no file provided. Using default...\n");
      } else {
        i++;
        path = argv[i];
      }
    } else if (streq(argv[i], "-t") || streq(argv[i], "--threads")) {
      int parsed = strtol(argv[i], NULL, 10);
      if (parsed == 0) {
	fprintf(stderr, "invalid format for threads. Using default (%d)...\n", CWEB_THREADS_DEFAULT);
      } else {
	threads = parsed;
      }
    }
  }
  cfg = httpserv_config_load(path);
  if (!cfg)
    exit(EXIT_FAILURE);
  // initalize logging system
  httpserv_logging_init("log.txt");
  tree_node_t *ip_node = tree_get_node(cfg, "http.ip");
  if (!ip_node) {
    httpserv_logging_err("no entry 'http.ip' in config file '%s'", path);
    cweb_cleanup();
    exit(EXIT_FAILURE);
  }
  tree_node_t *port_node = tree_get_node(cfg, "http.port");
  if (!port_node) {
    httpserv_logging_err("no entry 'http.port' in config file '%s'", path);
    cweb_cleanup();
    exit(EXIT_FAILURE);
  }
  char *ip = ip_node->value;
  int port = strtol(port_node->value, NULL, 10);
  if (!port) {
    httpserv_logging_err("'http.port' is invalid");
    cweb_cleanup();
    exit(EXIT_FAILURE);
  }
  srv = httpserv_httpserver_new(ip, port);
  int result = httpserv_httpserver_run(srv, threads);

  if (result != 0) return_value = EXIT_FAILURE;
  cweb_cleanup();
  return return_value;
}
