SPI (Serial Peripheral Interface)
-----------------------------------

The Serial Peripheral Interface (SPI) is a synchronous serial communication 
interface specification used for short distance communication, primarily in
embedded systems. The interface was developed by Motorola in the mid 1980s
and become a de facto standard.

SPI devices communicate in full duplex mode using a master-slave architecture
with a single master. The master device originates the frame for reading and
writing. Multiple slave devices are supported through selection with individual
slave select (CS) lines.

```
+------------------------+                    +------------------------+
|                  SCLK -|------------------->|  SCLK                  |
|  SPI             MOSI -|------------------->|  MOSI             SPI  |
| Master           MISO  |<-------------------|- MISO            Slave |
|                  CS^  -|------------------->|  SS^                   |
+------------------------+                    +------------------------+
```
# Usage

* Usage on uboot: uboot/README.md

* Usage on kernel: kernel/README.md

* Usage on userspace: user/README.md

# Reference

[Serial Peripheral Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)

[Using SPI with Linux](https://armbedded.taskit.de/node/318)
