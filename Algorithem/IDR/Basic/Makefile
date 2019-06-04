CC=gcc

CFLAGS = -I./

SRC := radix.c idr.c idr_run.c

# CONFIG
CFLAGS += -DCONFIG_BASE_SMALL=0

all: idr

idr: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o idr > /dev/null
