CC = gcc
CFLAGS = -std=c11 -Wall -I../include -Iinclude -I../libtree/include -ggdb
LDFLAGS = -L.. -lcmocka -lpthread -ltree -l:libhttpserv.a -lm
SRC = $(wildcard *.c) $(wildcard **/*.c)

all: mock

mock: $(SRC)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
