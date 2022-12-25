#ifndef __HTTPSERV_H__
#define __HTTPSERV_H__
#include "threadpool.h"
#include "job.h"
#include "config.h"
#include "logging.h"
#include "net/server.h"

// version macros
#define HTTPSERV_VERSION_MAJOR 0
#define HTTPSERV_VERSION_MINOR 1
#define HTTPSERV_VERSION_PATCH 0

// get the major runtime version
int httpserv_version_major();
// get the minor runtime version
int httpserv_version_minor();
// get the patch runtime version
int httpserv_version_patch();
#endif
