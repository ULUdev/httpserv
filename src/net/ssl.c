#include "net/ssl.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>

httpserv_ssl_t *httpserv_ssl_init(const char *privkeyfile, const char *certchainsfile) {
  if (!privkeyfile || !certchainsfile) return NULL;
  httpserv_ssl_t *ssl = malloc(sizeof(httpserv_ssl_t));
  ssl->certchains = malloc(strlen(certchainsfile)+1);
  ssl->privkey = malloc(strlen(privkeyfile)+1);
  strcpy(ssl->certchains, certchainsfile);
  strcpy(ssl->privkey, privkeyfile);
  return ssl;
}

void httpserv_ssl_destroy(httpserv_ssl_t *ssl) {
  free(ssl->certchains);
  free(ssl->privkey);
  SSL_free(ssl->ssl);
  SSL_CTX_free(ssl->ssl_ctx);
}
