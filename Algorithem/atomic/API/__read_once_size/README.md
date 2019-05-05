__read_once_size [中文教程](https://biscuitos.github.io/blog/LIST___read_once_size/)
----------------------------------

Read value from memory not cache nor register.

Context:

* Driver Files: atomic.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_XX) += atomic.o
```

Then, compile driver and dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
