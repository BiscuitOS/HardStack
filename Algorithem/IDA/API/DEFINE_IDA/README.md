DEFINE_IDA [中文教程](https://biscuitos.github.io/blog/IDA_DEFINE_IDA/)
----------------------------------

Allocate an used ID.

Context:

* Driver Files: ida.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_IDA_XX) += ida.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
