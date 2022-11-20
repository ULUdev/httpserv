#include "config.h"
#include "logging.h"
#include "node.h"
#include "tree.h"
#include "stringh.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
  char *label;
  char *value;
} httpserv_cfg_t;

httpserv_cfg_t *httpserv_config_process_line(const char *line,
                                             const size_t linenum) {
  httpserv_cfg_t *cfg = malloc(sizeof(httpserv_cfg_t));
  char *label = malloc(2);
  char *value = malloc(2);
  int label_read = 1;
  memset(label, '\0', 2);
  memset(value, '\0', 2);
  for (int i = 0; i < strlen(line); i++) {
    // comment
    if (line[i] == '#')
      break;
    if (label_read == 1) {
      if (line[i] == '=') {
        label_read = 0;
        continue;
      } else {
        label = strncat(label, &line[i], 1);
        label = realloc(label, strlen(label) + 2);
      }
    } else {
      value = strncat(value, &line[i], 1);
      value = realloc(value, strlen(value) + 2);
    }
  }
  if (label_read == 1) {
    free(label);
    free(value);
    free(cfg);
    return NULL;
  }
  httpserv_strstripltw(label);
  httpserv_strstripltw(value);
  label = realloc(label, strlen(label) + 1);
  value = realloc(value, strlen(value) + 1);
  cfg->label = label;
  cfg->value = value;
  return cfg;
}

tree_t *httpserv_config_load(const char *path) {
  FILE *conf_file = fopen(path, "r");
  if (!conf_file) {
    httpserv_logging_err("failed to open config file \"%s\": %s", path,
                         strerror(errno));
    return NULL;
  }
  tree_t *cfg_tree = tree_new();
  char *linebuf = malloc(3);
  memset(linebuf, '\0', 3);
  char charbuf;
  size_t linenum = 1;
  // int failed = 1;
  while (feof(conf_file) == 0) {
    size_t bytes_read = fread(&charbuf, 1, 1, conf_file);
    if (bytes_read == 0) {
      // httpserv_logging_err("failed to read bytes from stream");
      // failed = 0;
      break;
    }
    if (charbuf == '\n') {
      linebuf = strncat(linebuf, &charbuf, 1);
      httpserv_cfg_t *cfg_res = httpserv_config_process_line(linebuf, linenum);
      if (!cfg_res) {
        // line wasn't parsed properly
        continue;
      } else {
        tree_add_node(cfg_tree, cfg_res->label, cfg_res->value);
        free(cfg_res->label);
      }
      linenum++;
      memset(linebuf, '\0', strlen(linebuf) + 1);
    } else {
      linebuf = strncat(linebuf, &charbuf, 1);
    }
    linebuf = realloc(linebuf, strlen(linebuf) + 2);
  }
  free(linebuf);
  fclose(conf_file);
  // if (failed) {
  //   httpserv_config_destroy(cfg_tree);
  //   return NULL;
  // }
  return cfg_tree;
}

void httpserv_config_node_destroy(tree_node_t *node) {
  if (!node)
    return;
  free(node->value);
  for (int i = 0; i < node->numchildren; i++) {
    httpserv_config_node_destroy(node->children[i]);
  }
}

void httpserv_config_destroy(tree_t *cfg) {
  if (!cfg)
    return;
  httpserv_config_node_destroy(cfg->root);
  tree_destroy(cfg);
}
