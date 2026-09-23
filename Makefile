CC := cc
CPPFLAGS := -D_POSIX_C_SOURCE=200809L -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2

COMMON := src/common.c src/protocol.c

.PHONY: all clean test

all: bin/crypto-server bin/crypto-client bin/keygen

bin:
	mkdir -p bin

bin/crypto-server: bin src/server.c $(COMMON) include/common.h include/protocol.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/server.c $(COMMON) -o $@

bin/crypto-client: bin src/client.c $(COMMON) include/common.h include/protocol.h
	$(CC) $(CPPFLAGS) $(CFLAGS) src/client.c $(COMMON) -o $@

bin/keygen: bin src/keygen.c
	$(CC) $(CPPFLAGS) $(CFLAGS) src/keygen.c -o $@

test: all
	sh tests/integration.sh

clean:
	rm -rf bin
