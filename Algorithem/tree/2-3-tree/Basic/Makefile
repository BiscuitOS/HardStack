# Compile tools
CC=gcc

# Running source code
SRC = tree23_run.c tree23.c
# Performance test source code
SRC_PERF = tree23_perform.c tree23.c

# Cflags
CFLAGS := -I./

# Target file
TARGET := tree23 tree23_test

all: $(TARGET)

tree23: $(SRC)
	@gcc $(SRC) $(CFLAGS) -o $@ 
tree23_test: 
	@gcc $(SRC_PERF) $(CFLAGS) -o $@ 
clean:
	@rm $(TARGET)
