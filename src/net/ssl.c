#include "net/ssl.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>

httpserv_ssl_t *httpserv_ssl_init(const char *privkeyfile,
                                  const char *certfile) {
  if (!privkeyfile || !certfile)
    return NULL;
  httpserv_ssl_t *ssl = malloc(sizeof(httpserv_ssl_t));
  ssl->certchains = malloc(strlen(certfile) + 1);
  ssl->privkey = malloc(strlen(privkeyfile) + 1);
  strcpy(ssl->certchains, certfile);
  strcpy(ssl->privkey, privkeyfile);
  ssl->ssl_ctx = SSL_CTX_new(TLS_method());
  if (SSL_CTX_use_certificate_file(ssl->ssl_ctx, certfile, SSL_FILETYPE_PEM) !=
      1) {
    httpserv_ssl_destroy(ssl);
    return NULL;
  }
  if (SSL_CTX_use_PrivateKey_file(ssl->ssl_ctx, privkeyfile,
                                  SSL_FILETYPE_PEM) != 1) {
    httpserv_ssl_destroy(ssl);
    return NULL;
  }
  if (!SSL_CTX_check_private_key(ssl->ssl_ctx)) {
    httpserv_logging_err("private key and public certificate don't match");
    httpserv_ssl_destroy(ssl);
    return NULL;
  }
  ssl->ssl = SSL_new(ssl->ssl_ctx);
  return ssl;
}
httpserv_ssl_t *httpserv_ssl_init_conn(SSL_CTX *ctx, const int fd) {
  httpserv_ssl_t *ssl = malloc(sizeof(httpserv_ssl_t));
  ssl->ssl_ctx = ctx;
  ssl->ssl = SSL_new(ssl->ssl_ctx);
  SSL_set_fd(ssl->ssl, fd);
  return ssl;
}

void httpserv_ssl_destroy(httpserv_ssl_t *ssl) {
  SSL_shutdown(ssl->ssl);
  SSL_read(ssl->ssl, NULL, 0);
  int fd = SSL_get_fd(ssl->ssl);
  close(fd);
  free(ssl->certchains);
  free(ssl->privkey);
  SSL_free(ssl->ssl);
  SSL_CTX_free(ssl->ssl_ctx);
}
