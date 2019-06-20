\_\_set_bit [中文教程](https://biscuitos.github.io/blog/BITMAP___set_bit/)
----------------------------------

Set a bit in memory.

Context:

* Driver Files: bitmap.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_BITMAP_XX) += bitmap.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
