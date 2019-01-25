klist_remove [中文教程](https://biscuitos.github.io/blog//)
----------------------------------

Decreament the refcount of node and wati for it to go away.

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
