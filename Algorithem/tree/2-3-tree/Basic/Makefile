CC=gcc

CFLAGS = -I./

SRC := tree23.c tree23_usage.c

all: tree23

tree23: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o tree23 > /dev/null
