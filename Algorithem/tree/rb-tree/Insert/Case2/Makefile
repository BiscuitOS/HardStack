CC=gcc

CFLAGS = -I./

SRC := rbtree.c rb_run.c

all: rbtree

rbtree: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	@rm -rf *.o rbtree > /dev/null
