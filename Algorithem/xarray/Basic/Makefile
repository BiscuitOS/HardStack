# SPDX-License-Identifier: GPL-2.0+ 
CC=gcc

CFLAGS = -I./

SRC := xarray.c xarray_run.c

# Config
CFLAGS += -DCONFIG_BASE_SMALL=0

all: xarray

xarray: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o xarray > /dev/null
