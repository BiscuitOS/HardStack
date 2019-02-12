of_find_matching_node_and_match [中文教程](https://biscuitos.github.io/blog/DTS-of_find_matching_node_and_match/)
----------------------------------

Find a device node and device id via a device id list.

Context:

* DTS Files: demo.dtsi

* Driver Files: demo.c

## Usage

Copy DTS Files into `/arch/arm64/boot/dts`, and modify core specifical DTS 
file as follow:

```
include "demo.dtsi"
```

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-y += demo.o
```

Then, compile driver and dts. Details :

```
make
make dtbs
```

## Running

Packing image and runing on the target board.

