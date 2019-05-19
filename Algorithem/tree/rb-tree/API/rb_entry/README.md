rb_entry [中文教程](https://biscuitos.github.io/blog/RBTREE_rb_entry/)
----------------------------------

obtain struct that embeed rb_node.

Context:

* Driver Files: rbtree.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_RBTREE_XX) += rbtree.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
