atomic_cmpxchg [中文教程](https://biscuitos.github.io/blog/ATOMIC_atomic_cmpxchg/)
----------------------------------

Compare and exchange data.

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
