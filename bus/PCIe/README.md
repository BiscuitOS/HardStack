PCIe                             [中文](https://biscuitos.github.io/blog/PCIe/)
-----------------------------------------

![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000001.png)

### Usage of PCIe driver:

Target: Intel Ubuntu 14.04 64-Bit

Linux: 3.19.0-25-generic

Porting PCIe driver into target PC, and running follow command:

```
cd drv
make clean
make
insmod pcie.ko
```

![PCIe Driver](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000000.png)
