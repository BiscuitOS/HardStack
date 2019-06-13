# SPDX-License-Identifier: GPL-2.0
CC=gcc

CFLAGS = -I./

# Config

## PORT
CFLAGS += -DCONFIG_PORT=8860

all: client server

client: tcp_client.c
	@$(CC) $^ $(CFLAGS) -o $@

server: tcp_server.c
	@$(CC) $^ $(CFLAGS) -o $@

clean:
	@rm -rf *.o client server > /dev/null
