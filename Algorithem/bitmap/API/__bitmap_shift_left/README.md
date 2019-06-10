\_\_bitmap_shift_left [中文教程](https://biscuitos.github.io/blog/BITMAP___bitmap_shift_left/)
----------------------------------

Logical left shift of the bits in a bitmap.

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
