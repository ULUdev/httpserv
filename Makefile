CC = gcc
LD = ld
AR = ar
CFLAGS = -fPIC -c -std=c11 -Wall -Iinclude -Ilibtree/include -O3
LDFLAGS = -shared -L -ltree -lpthread -lm
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)
PREFIX = /usr/local

all: libtree.a libhttpserv.so libhttpserv.a cweb

libtree.a:
	$(MAKE) -C libtree
	cp libtree/libtree.a .

cweb: cweb.c libhttpserv.so
	$(CC) cweb.c -std=c11 -Wall -O3 -Iinclude -Ilibtree/include -L. -lpthread -ltree -lhttpserv -lm -o cweb

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

tests: libtree.a libhttpserv.so test/mock
	LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$$PWD" ./test/mock

install: cweb libhttpserv.so
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib
	mv cweb $(PREFIX)/bin
	mv libhttpserv.so $(PREFIX)/lib

test/mock: $(wildcard test/*.c)
	$(MAKE) -C test
