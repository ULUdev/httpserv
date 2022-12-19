#include "bufferpool.h"
#include <bits/pthreadtypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define BIT_CHECK(var, mask) ((var & mask) == mask)

httpserv_buffer_t *httpserv_buffer_new(const size_t size,
                                       const httpserv_buffer_flags_t flags) {
  httpserv_buffer_t *buf = malloc(sizeof(httpserv_buffer_t));
  buf->mutex = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(buf->mutex, NULL) != 0) {
    free(buf->mutex);
    free(buf);
    return NULL;
  }
  buf->content = malloc(size);
  buf->size = size;
  buf->pos = 0;
  buf->next = NULL;
  buf->flags = flags;
  return buf;
}
size_t httpserv_buffer_write(httpserv_buffer_t *buf,
                             const httpserv_byte_t *content, const size_t len) {
  size_t written = 0;
  size_t to_write = len;
  size_t pos = 0;
  httpserv_buffer_t *curbuf = buf;
  while (to_write > 0) {
    if (!curbuf)
      break;
    pthread_mutex_lock(curbuf->mutex);
    // add a new buffer if the current one is configured in such a way
    if (!curbuf->next && to_write > curbuf->size &&
        BIT_CHECK(curbuf->flags, HTTPSERV_BUFFER_FLAGS_CRLINK)) {
      int tmp = !BIT_CHECK(curbuf->flags, HTTPSERV_BUFFER_FLAGS_CRLINK_NO_TEMP);
      httpserv_buffer_flags_t tmpflags =
          tmp ? HTTPSERV_BUFFER_FLAGS_TEMP : HTTPSERV_BUFFER_FLAGS_NONE;
      httpserv_buffer_t *crlink_buf =
          httpserv_buffer_new(curbuf->size, curbuf->flags | tmpflags);
      if (!crlink_buf)
        break;
      else
        curbuf->next = crlink_buf;
    }
    memcpy(curbuf->content, content + pos,
           curbuf->size > to_write ? to_write : curbuf->size);
    if (curbuf->size > to_write) {
      to_write = 0;
      written += to_write;
    } else {
      to_write -= curbuf->size;
      written += curbuf->size;
      pos += curbuf->size;
    }
    pthread_mutex_unlock(curbuf->mutex);
    curbuf = curbuf->next;
  }
  return written;
}
size_t httpserv_buffer_writeat(httpserv_buffer_t *buf, size_t _pos,
                               const httpserv_byte_t *content,
                               const size_t len) {
  size_t written = 0;
  size_t to_write = len;
  size_t pos = 0;
  httpserv_buffer_t *curbuf = buf;
  while (to_write > 0) {
    if (!curbuf)
      break;
    pthread_mutex_lock(curbuf->mutex);
    // add a new buffer if the current one is configured in such a way
    if (!curbuf->next && to_write > curbuf->size &&
        BIT_CHECK(curbuf->flags, HTTPSERV_BUFFER_FLAGS_CRLINK)) {
      int tmp = !BIT_CHECK(curbuf->flags, HTTPSERV_BUFFER_FLAGS_CRLINK_NO_TEMP);
      httpserv_buffer_flags_t tmpflags =
          tmp ? HTTPSERV_BUFFER_FLAGS_TEMP : HTTPSERV_BUFFER_FLAGS_NONE;
      httpserv_buffer_t *crlink_buf =
          httpserv_buffer_new(curbuf->size, curbuf->flags | tmpflags);
      if (!crlink_buf)
        break;
      else
        curbuf->next = crlink_buf;
    }
    memcpy(curbuf->content, content + pos,
           curbuf->size > to_write ? to_write : curbuf->size);
    if (curbuf->size > to_write) {
      to_write = 0;
      written += to_write;
    } else {
      to_write -= curbuf->size;
      written += curbuf->size;
      pos += curbuf->size;
    }
    pthread_mutex_unlock(curbuf->mutex);
    curbuf = curbuf->next;
  }
  return written;
}
size_t httpserv_buffer_read(httpserv_buffer_t *buf, httpserv_byte_t *ubuf,
                            const size_t len) {
  size_t read = 0;
  size_t to_read = len;
  size_t pos = 0;
  httpserv_buffer_t *curbuf = buf;
  while (curbuf->pos == curbuf->size) {
    if (curbuf->next)
      curbuf = curbuf->next;
    else
      return read;
  }
  while (to_read > 0) {
    if (!curbuf)
      break;
    memcpy(ubuf + pos, curbuf->content + curbuf->pos,
           to_read > curbuf->size - curbuf->pos ? curbuf->size - curbuf->pos
                                                : to_read);
    if (to_read > curbuf->size) {
      pos += curbuf->size-curbuf->pos;
      curbuf->pos += curbuf->size-curbuf->pos;
      curbuf = curbuf->next;
      to_read -= curbuf->size-curbuf->pos;
    } else {
      pos += to_read;
      curbuf->pos += to_read;
    }
  }
  return read;
}
void httpserv_buffer_rewind(httpserv_buffer_t *buf) {
  buf->pos = 0;
  if (BIT_CHECK(buf->flags, HTTPSERV_BUFFER_FLAGS_NO_REWIND_NEXT) && buf->next)
    httpserv_buffer_rewind(buf->next);
}
void httpserv_buffer_destroy(httpserv_buffer_t *buf) {
  pthread_mutex_destroy(buf->mutex);
  free(buf->mutex);
  free(buf->content);
  free(buf);
}

httpserv_bufferpool_t *
httpserv_bufferpool_new(const size_t bufsiz, const size_t init_buffers,
                        const httpserv_buffer_flags_t flags) {
  httpserv_bufferpool_t *pool = malloc(sizeof(httpserv_bufferpool_t));
  pool->mutex = malloc(sizeof(pthread_mutex_t));
  if (pthread_mutex_init(pool->mutex, NULL) != 0) {
    free(pool->mutex);
    free(pool);
    return NULL;
  }
  pool->bufsiz = bufsiz;
  pool->head = NULL;
  pool->tail = NULL;
  pool->flags = flags;
  pool->len = 0;
  pool->mutex = malloc(sizeof(pthread_mutex_t));
  return pool;
}
httpserv_buffer_t *httpserv_bufferpool_get(httpserv_bufferpool_t *pool,
                                           const size_t size) {
  if (!pool->head) return NULL;
  pthread_mutex_lock(pool->mutex);
  size_t size_retrieved = size;
  httpserv_buffer_t *buf = pool->head;
  while (size_retrieved < size) {
    if (!buf->next) return NULL;
    size_retrieved += buf->size;
    buf = buf->next;
  }
  buf->next = NULL;
  pthread_mutex_unlock(pool->mutex);
  return buf;
}
void httpserv_bufferpool_release(httpserv_bufferpool_t *pool,
                                 httpserv_buffer_t *buf) {
  if (!buf) return;
  pthread_mutex_lock(pool->mutex);
  if (!pool->tail) {
    pool->tail = buf;
    pool->head = buf;
  } else {
    pool->tail->next = buf;
    httpserv_buffer_t *curbuf = buf;
    while (curbuf->next) {
      curbuf = curbuf->next;
    }
    pool->tail = curbuf;
  }
  pthread_mutex_unlock(pool->mutex);
}
void httpserv_bufferpool_destroy(httpserv_bufferpool_t *pool) {
  pthread_mutex_destroy(pool->mutex);
  free(pool->mutex);
  if (pool->head) {
    httpserv_buffer_t *curbuf = pool->head;
    while (curbuf) {
      httpserv_buffer_t *nextbuf = curbuf->next;
      httpserv_buffer_destroy(curbuf);
      curbuf = nextbuf;
    }
  }
  free(pool);
}
