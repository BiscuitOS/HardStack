BiscuitOS QEMU Debug Stub Usage
======================================

#### Deploy

1. Goto Qemu source code directory

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
```

**VERSION** is kernel version and **ARCH** is architecture of platformat. qemu-system is soft link from variable version qemu.

2. Download Stub source code

We need download and copy source code from BiscuitOS, sach as:

```
cd BiscuitOS/
make linux-5.0-x86_64_defconfig
make menuconfig 

  [*] Package --->
      [*] BiscuitOS Debug Stub Set --->
          [*] BiscuitOS QEMU Debug Stub --->

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-QEMU-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-QEMU-default/BiscuitOS-debug-stub-QEMU-default/BiscuitOS-stub.c ./ -rf
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-QEMU-default/BiscuitOS-debug-stub-QEMU-default/BiscuitOS-stub.h ./include/ -rf
```

And the modify Makefile.objs under current directory, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
vi Makefile.objs

# Add context
common-obj-y += BiscuitOS-stub.o

cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/include/
vi qemu-common.h

# Add context
+ #include "BiscuitOS-stub.h"
```

3. Compile QEMU

BiscuitOS provides a auto scripts to compile QEMU, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/
./RunQEMU.sh -b
```

4. usage

**bs_debug_enable()** will enable print infomation from **bs_debug(...)**, and **bs_debug_disable()** will disable print infomation.

#### Blog

```
https://biscuitos.github.io/blog/BiscuitOS_Catalogue/
```
