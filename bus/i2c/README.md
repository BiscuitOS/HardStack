I2C  [中文](https://biscuitos.github.io/blog/I2CBus/)
-------------------------------------------------

![i2c_timing](https://github.com/EmulateSpace/PictureSet/blob/master/i2c/i2cdebug.png)

I²C (Inter-Integrated Circuit), pronounced I-squared-C, is a multi-master, 
multi-slave, packet switched, single-ended, serial computer bus invented by 
Philips Semiconductor (now NXP Semiconductors). It is typically used for 
attaching lower-speed peripheral ICs to processors and microcontrollers in 
short-distance, intra-board communication. Alternatively I²C is spelled I2C 
(pronounced I-two-C) or IIC (pronounced I-I-C).

SMBus, defined by Intel in 1995, is a subset of I²C, defining a stricter 
usage. One purpose of SMBus is to promote robustness and interoperability. 
Accordingly, modern I²C systems incorporate some policies and rules from 
SMBus, sometimes supporting both I²C and SMBus, requiring only minimal 
reconfiguration either by commanding or output pin use.

## Usage

* Usage on uboot: uboot/README.md

* Usage on kernel: kernel/README.md

* Usage on userspace: user/README.md
