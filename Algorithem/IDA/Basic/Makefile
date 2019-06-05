CC=gcc

CFLAGS = -I./

SRC := radix.c ida.c ida_run.c

# CONFIG
CFLAGS += -DCONFIG_BASE_SMALL=0

all: ida

ida: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o ida > /dev/null
