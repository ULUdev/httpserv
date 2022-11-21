#include "config.h"
#include "logging.h"
#include "threadpool.h"
#include "stringh.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#define streq(s1, s2) (httpserv_streq(s1, s2) == 0)

#define CWEB_VERSION_MAJOR 0
#define CWEB_VERSION_MINOR 1
#define CWEB_VERSION_PATCH 0

const char *CWEB_HELP_STR =
    "\n"
    "OPTIONS\n"
    "  -h,--help: print this help and exit\n"
    "  -v,--version: print the version number and exit\n"
    "  -f,--file <file>: use <file> instead of the default configuration "
    "file\n";

int main(int argc, char **argv) {
  char *path = "/etc/cweb/cweb.conf";
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
    }
  }
  // initalize logging system
  tree_t *cfg = httpserv_config_load(path);
  httpserv_logging_init("log.txt");
  // end logging system
  httpserv_config_destroy(cfg);
  httpserv_logging_destroy();
  return 0;
}
