idr_for_each_entry [中文教程](https://biscuitos.github.io/blog/IDR_idr_for_each_entry/)
----------------------------------

Iterate over all node on IDR.

Context:

* Driver Files: idr.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_IDR_XX) += idr.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
