MDIO (SMI/MIIM) on Kernel
-------------------------------------

MDIO is subset of MII that exchange data between MAC and PHY. On mido bus 
subsystem, the mdio bus alway be contained on PHY or Switch driver. Kernel 
offer some API to register a mdio bus to PHY or Switch.

Developer alway offer MDIO read and write operation on MDIO controller. Some 
useful API as follow:

* midobus_alloc

  Allocate memory to a mdio bus.

* mdiobus_free

  Release mdio bus memory into system.

* mdiobus_register

  Register a mdio bus into system.

* mdiobus_unregister

  Unregister a mdio bus from system.

## Usage

```
make clean
make
sudo insmod mdio.ko
```
