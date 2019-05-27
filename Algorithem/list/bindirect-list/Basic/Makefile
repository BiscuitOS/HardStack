CC=gcc

CFLAGS = -I./

SRC := list_run.c

all: list

list: $(SRC)
	@$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o list > /dev/null
