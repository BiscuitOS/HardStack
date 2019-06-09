BITMAP_FIRST_WORD_MASK [中文教程](https://biscuitos.github.io/blog/BITMAP_BITMAP_FIRST_WORD_MASK/)
----------------------------------

Clear bit area from first bit on Bitmap.

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
