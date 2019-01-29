Platform Bus device driver [中文教程](https://biscuitos.github.io/blog//)
----------------------------------

Platform device driver mini.

Context:

* Driver Files which using DTS: of_demo.c

* Driver Files which doesn't use DTS: desc_demo.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-y += of_demo.o
or
obj-y += desc_demo.o
```

If using DTS mechanism, copy DTS Files into `/arch/arm64/boot/dts`, and modify core specifical DTS file as follow:

```
include "demo.dtsi"
```

Then, compile driver and dts. Details :

```
make
make dtbs
```

## Running

Packing image and runing on the target board.
