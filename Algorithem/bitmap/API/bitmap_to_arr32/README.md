bitmap_to_arr32 [中文教程](https://biscuitos.github.io/blog/BITMAP_bitmap_to_arr32/)
----------------------------------

Cover Bitmap into 32-bit array.

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
