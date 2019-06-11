\_\_bitmap_subset [中文教程](https://biscuitos.github.io/blog/BITMAP___bitmap_subset/)
----------------------------------

Check whether bitmap1 is subset of bitmap2 on special bits.

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
