sysfs_create_link [中文教程](https://biscuitos.github.io/blog//)
----------------------------------

Create a soft-link under special directory on sysfs.

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
