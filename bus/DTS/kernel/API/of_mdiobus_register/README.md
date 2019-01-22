of_mdiobus_register [中文教程](https://biscuitos.github.io/blog/DTS-of_mdiobus_register/)
----------------------------------

Register a MDIO bus and probe PHY device into system.

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

