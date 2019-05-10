BBXX [中文教程](https://biscuitos.github.io/blog/XARRAY_BBXX/)
----------------------------------

XARRAY.

Context:

* Driver Files: xarray.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += xarray.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
