CC=gcc
LD= ld
CFLAGS= -std=c11 -Wall -ggdb -Iinclude -ggdb
LDFLAGS= -lpthread
SRC= $(wildcard src/**/*.c) $(wildcard src/*.c)

all: httpserv

httpserv: $(SRC)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf httpserv $(subst .c,,$(wildcard test/*.c))

format:
	clang-format -i $(SRC)

tests: $(subst .c,,$(wildcard test/*.c))

test/%: test/%.c
	$(CC) $(SRC) $^ -DHTTPSERV_UNIT_TEST $(CFLAGS) $(LDFLAGS) -lcmocka -o $@
	./$@
