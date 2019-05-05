WRITE_ONCE [中文教程](https://biscuitos.github.io/blog/LIST_WRITE_ONCE/)
----------------------------------

Write dato into memory not cache/buffer/register.

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
