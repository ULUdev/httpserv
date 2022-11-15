CC=gcc
LD= ld
CFLAGS= -Wall -ggdb -Iinclude -Ithreadpool/include -ggdb
LDFLAGS= -lpthread
SRC= $(wildcard src/**/*.c) $(wildcard src/*.c)

all: httpserv

httpserv: $(SRC)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf lib httpserv
	$(MAKE) -C threadpool clean

