# SPDX-License-Identifier: GPL-2.0
CC=gcc

# Architecture
ARCH := $(shell getconf LONG_BIT)

CFLAGS = -I./

SRC := bitmap.c bitmap_run.c

# Config

ifeq ($(ARCH),64)
CFLAGS += -DCONFIG_64BIT
endif

all: bitmap

bitmap: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o bitmap > /dev/null
