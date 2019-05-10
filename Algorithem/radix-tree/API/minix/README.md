BBXX [中文教程](https://biscuitos.github.io/blog/RADIX-TREE_BBXX/)
----------------------------------

Radix-tree.

Context:

* Driver Files: radix.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += radix.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
