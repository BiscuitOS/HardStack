of_find_device_by_node
----------------------------------

Obtain platform device by device node.

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

