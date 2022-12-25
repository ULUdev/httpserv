CC = gcc
ENV = TEST_ENV_NORMAL
CFLAGS = -D$(ENV) -std=c11 -Wall -I../include -I../libtree/include -ggdb
LDFLAGS = -L.. -lcmocka -lpthread -ltree -l:libhttpserv.a -lm $(shell pkg-config --libs openssl)
SRC = $(wildcard *.c) $(wildcard **/*.c)

all: mock

mock: $(SRC)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
