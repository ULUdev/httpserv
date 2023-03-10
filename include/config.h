#ifndef __HTTPSERV_CONFIG_H__
#define __HTTPSERV_CONFIG_H__
#include "tree.h"

tree_t *httpserv_config_load(const char *path);
/*
 * This function is only to be used if `cfg` was generated by
 * `httpserv_config_load`
 */
void httpserv_config_destroy(tree_t *cfg);
#endif
