CC = gcc
LD = ld
CFLAGS = -fPIC -c -std=c11 -Wall -Iinclude -Ilibtree/include -ggdb
LDFLAGS = -shared -L. -ltree -lpthread
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)
PREFIX = /usr/local

all: libtree.so libhttpserv.so cweb

libtree.so:
	$(MAKE) -C libtree
	cp libtree/libtree.so .

cweb: cweb.c libhttpserv.so
	$(CC) cweb.c -std=c11 -Wall -ggdb -Iinclude -L. -lpthread -ltree -lhttpserv -o cweb

libhttpserv.so: $(SRC:.c=.o)
	$(LD) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf httpserv test/mock $(SRC:.c=.o) cweb

format:
	clang-format -i $(SRC) $(wildcard test/*.c) cweb.c

tests: libhttpserv.so test/mock
	LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$$PWD" ./test/mock


test/mock: $(wildcard test/*.c)
	$(MAKE) -C test
