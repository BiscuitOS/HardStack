\_\_bitmap_parse [中文教程](https://biscuitos.github.io/blog/BITMAP___bitmap_parse/)
----------------------------------

convert an ASCII hex string into a bitmap.

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
