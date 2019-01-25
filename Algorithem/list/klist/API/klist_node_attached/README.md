klist_node_attached [中文教程](https://biscuitos.github.io/blog//)
----------------------------------

Say whether a node is bound to a list or not.

Context:

* Driver Files: demo.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-y += demo.o
```

Then, compile driver and dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
