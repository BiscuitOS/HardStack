\_\_cond_lock [中文教程](https://biscuitos.github.io/blog/SPARSE___cond_lock/)
----------------------------------

Lock with condition.

Context:

* Driver Files: sparse.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPARSE_XX) += sparse.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
