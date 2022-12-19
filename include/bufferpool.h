#ifndef __HTTPSERV_BUFFERPOOL_H__
#define __HTTPSERV_BUFFERPOOL_H__
#include "data.h"
#include <bits/pthreadtypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * Type used by flags passed to buffers
 */
typedef uint8_t httpserv_buffer_flags_t;
// no flags at all
#define HTTPSERV_BUFFER_FLAGS_NONE 0b00000000

// buffers next pointer may be used
#define HTTPSERV_BUFFER_FLAGS_LINK 0b00000001

// if the buffer is not large enough, a buffer of the same size may be created
// and linked to. All new buffers will have the TEMP flag set
#define HTTPSERV_BUFFER_FLAGS_CRLINK 0b00000010

// buffer is only temporary. When released into bufferpool it will be
// deallocated
#define HTTPSERV_BUFFER_FLAGS_TEMP 0b00000100

// buffer can be resized if needed. This stands in contrast to the CRLINK flag
#define HTTPSERV_BUFFER_FLAGS_RESIZE 0b00001000

// new buffers created by CRLINK will not be temporary. This flag is ignored if
// CRLINK is not set
#define HTTPSERV_BUFFER_FLAGS_CRLINK_NO_TEMP 0b00010000

// when rewinding a buffer the next pointer is never rewinded
#define HTTPSERV_BUFFER_FLAGS_NO_REWIND_NEXT 0b00100000

/*
 * A thread-safe buffer usually owned by a pool. It's next pointer holds the
 * next buffer, if multiple buffers are linked together. This can occur if a
 * buffer of some large size is needed to hold data. A buffer is to be only
 * owned by a single thread. After use it has to be either deallocated or
 * released to the pool.
 */
typedef struct HttpservBuffer {
  httpserv_byte_t *content;
  size_t size;
  size_t pos;
  httpserv_buffer_flags_t flags;
  struct HttpservBuffer *next;
  pthread_mutex_t *mutex;
} httpserv_buffer_t;

/*
 * A thread-safe pool of buffers. The number of buffers owned by the pool may
 * change. You should pick a reasonable default set of buffers.
 */
typedef struct {
  httpserv_buffer_t *head;
  httpserv_buffer_t *tail;
  httpserv_buffer_flags_t flags;
  size_t len;
  size_t bufsiz;
  pthread_mutex_t *mutex;
} httpserv_bufferpool_t;

httpserv_buffer_t *httpserv_buffer_new(const size_t size,
                                       const httpserv_buffer_flags_t flags);
/*
 * This function returns the number of bytes written into the buffer. 0 is
 * returned, if an error occured or `len` was 0. If for some reason it wasn't
 * abled to write all the data the returned value will be smaller than `len`
 */
size_t httpserv_buffer_write(httpserv_buffer_t *buf,
                             const httpserv_byte_t *content, const size_t len);
// same as httpserv_buffer_write except that we start writing at `_pos`
size_t httpserv_buffer_writeat(httpserv_buffer_t *buf, size_t _pos,
                               const httpserv_byte_t *content,
                               const size_t len);
/*
 * Read `len` bytes from `buf` into `ubuf`. In order to keep the improvements of
 * bufferpool don't overuse this function. Instead sometimes you might want to
 * access the struct member `content`.
 */
size_t httpserv_buffer_read(httpserv_buffer_t *buf, httpserv_byte_t *ubuf,
                            const size_t len);
// rewind a buffer (for reading only)
void httpserv_buffer_rewind(httpserv_buffer_t *buf);
void httpserv_buffer_destroy(httpserv_buffer_t *buf);

httpserv_bufferpool_t *
httpserv_bufferpool_new(const size_t bufsiz, const size_t init_buffers,
                        const httpserv_buffer_flags_t flags);
httpserv_buffer_t *httpserv_bufferpool_get(httpserv_bufferpool_t *pool,
                                           const size_t size);
void httpserv_bufferpool_release(httpserv_bufferpool_t *pool,
                                 httpserv_buffer_t *buf);
void httpserv_bufferpool_destroy(httpserv_bufferpool_t *pool);
#endif
