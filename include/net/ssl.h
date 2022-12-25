#ifndef __HTTPSERV_SSL_H__
#define __HTTPSERV_SSL_H__
#include <openssl/ssl.h>

/*
 * A struct representing all the information needed for httpserv to handle ssl
 * connections
 */
typedef struct {
  SSL_CTX *ssl_ctx;
  SSL_METHOD *method;
  SSL *ssl;
  char *certchains;
  char *privkey;
} httpserv_ssl_t;

httpserv_ssl_t *httpserv_ssl_init(const char *privkeyfile,
                                  const char *certchainsfile);
httpserv_ssl_t *httpserv_ssl_init_conn(SSL_CTX *ctx, const int fd);
void httpserv_ssl_destroy(httpserv_ssl_t *ssl);

#endif
