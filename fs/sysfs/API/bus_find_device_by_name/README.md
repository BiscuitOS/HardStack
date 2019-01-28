bus_find_device_by_name [中文教程](https://biscuitos.github.io/blog//)
----------------------------------

Find device by name on special bus.

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
