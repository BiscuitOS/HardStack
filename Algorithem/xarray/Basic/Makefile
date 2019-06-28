# SPDX-License-Identifier: GPL-2.0+ 
CC=gcc

# Architecture
ARCH := $(shell getconf LONG_BIT)

CFLAGS = -I./

SRC := xarray.c xarray_run.c

# Bitmap
SRC += bitmap.c

# Config
CFLAGS += -DCONFIG_BASE_SMALL=0

ifeq ($(ARCH),64)
CFLAGS += -DCONFIG_64BIT
endif

all: xarray

xarray: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o xarray > /dev/null
