CC = gcc
LD = ld
AR = ar
CFLAGS = -fPIC -c -std=c11 -Wall -Iinclude -Ilibtree/include -ggdb
LDFLAGS = -shared -L. -ltree -lpthread -lm $(shell pkg-config --libs openssl)
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)
PREFIX = /usr/local

all: libtree.a libhttpserv.so libhttpserv.a cweb

libtree.a:
	$(MAKE) -C libtree libtree.a
	cp libtree/libtree.a .

cweb: cweb.c libhttpserv.so
	$(CC) cweb.c -std=c11 -Wall -ggdb -Iinclude -Ilibtree/include -L. -lpthread -ltree -lhttpserv -lm $(shell pkg-config --libs openssl) -o cweb

libhttpserv.so: $(SRC:.c=.o)
	$(LD) $^ $(LDFLAGS) -o $@

libhttpserv.a: $(SRC:.c=.o)
	$(AR) -crs $@ $^

%.o: %.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf libhttpserv.so test/mock $(SRC:.c=.o) cweb libtree.a libhttpserv.a

format:
	clang-format -i $(SRC) $(wildcard test/*.c) $(wildcard include/*.h) $(wildcard include/**/*.h) cweb.c

tests: libtree.a libhttpserv.a test/mock
	LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$$PWD" ./test/mock

install: cweb libhttpserv.so
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib
	mv cweb $(PREFIX)/bin
	mv libhttpserv.so $(PREFIX)/lib

test/mock: $(wildcard test/*.c)
	$(MAKE) -C test
