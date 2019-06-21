\_\_\_\_atomic_change_bit [中文教程](https://biscuitos.github.io/blog/BITMAP_____atomic_change_bit/)
----------------------------------

Change a bit with atomic.

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
