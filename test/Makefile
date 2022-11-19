CC = gcc
CFLAGS = -std=c11 -Wall -I../include -Iinclude -ggdb
LDFLAGS = -L.. -lcmocka -lpthread -lhttpserv
SRC = $(wildcard *.c)

all: mock

mock: $(SRC)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@
