CC = gcc
LD = ld
CFLAGS = -fPIC -c -std=c11 -Wall -Iinclude -ggdb
LDFLAGS = -shared -lpthread
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)

all: libhttpserv.so cweb

cweb: cweb.c libhttpserv.so
	$(CC) cweb.c -std=c11 -Wall -ggdb -Iinclude -L. -lpthread -lhttpserv -o cweb

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
