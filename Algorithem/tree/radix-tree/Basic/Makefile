CC=gcc

CFLAGS = -I./

SRC := radix.c radix_run.c

# CONFIG
CFLAGS += -DCONFIG_BASE_SMALL=0

all: radix

radix: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o radix > /dev/null
