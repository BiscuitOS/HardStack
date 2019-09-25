Basic Platform Usermanual
-------------------------------------------

```
=====================================
          Platform Bus
=====================================
     A           A           A
     |           |           |
     V           V           V
+---------+ +---------+ +---------+
| Device0 | | Device1 | | Device2 | ..
+---------+ +---------+ +---------+
                 A
                 |
                 V
            +---------+
            | Driver0 |
            +---------+
```

> - [File list](#A01)
>
> - [Linux Device Driver](#A00)
>
> - [Userland](#A02)


-------------------------------------------

## <span id="A01">File list</span>

* kernel/platform.c

  The platfrom device driver file which contain core function
  for special device.

* kernel/Makefile

  The Makefile for module. If you want to build module or build
  on extral source tree, the Makefile will build module for platform.c

* kernel/Kconfig

  This file describe how to add device driver into Kbuild system.

* userland/platform.c

  This C source code to access platform device on userland.

* README.md

  Default Usermanual.

--------------------------------------------

## <span id="A00">Linux Device Driver</span>

This is basic platform device driver with DTS. It support
probe(), remove(), suspend(), resume(), and shutdown()
interface. This project support build on linux source
tree, and also support extral module build.

--------------------------------------------

## Usage

The driver support build in kernel or build as module. You can do as
you do as follow:

> - [Build in kernel](#B00)
>
> - [Build as module in kernel](#B01)
>
> - [Build as module extral](#B02)

----------------------------------------

#### <span id="B00">Build in kernel</span>

Build driver as part of Kernel image with object, so you should
follow steps:

1. Copy dirver onto special directory on kernel source tree, as:

   ```
   cp platform.c BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS
   ```

2. Add Kconfig item into Kbuild system, such as

   ```
   vi BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS/Kconfig

   modify Kconfig:

   config PLATFORM_LDD_BASE
	tristate "Platform LDD base"   
   ```

3. Add object onto Makefile, such

   ```
   vi BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS/Makefile
   
   modify Makefile:

   obj-$(CONFIG_PLATFORM_LDD_BASE) += platform.o
   ```

4. Configure Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32/linux/linux/
   make ARCH=arm memuconfig

   Device Driver --->
      BiscuitOS LDD --->
          <*> Platform LDD base
   ```
5. Compile Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32/linux/linux/
   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -j8
   ```

6. Running Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32
   ./RunBiscuitOs.sh
   ```

7. Review module on /proc

   ```
   Logon on BiscuitOS
   ~# cd /sys/devices/platform/Platform_demo
   ~# ls
   driver           modalias         subsystem
   driver_override  power            uevent
   ```
---------------------------------

#### <span id="B01">Build as module in kernel</span>

Build driver as module on kernel source tree. You can follow steps:


1. Copy dirver onto special directory on kernel source tree, as:

   ```
   cp platform.c BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS
   ```

2. Add Kconfig item into Kbuild system, such as

   ```
   vi BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS/Kconfig

   modify Kconfig:

   config PLATFORM_LDD_BASE
	tristate "Platform LDD base"   
   ```

3. Add object onto Makefile, such

   ```
   vi BiscuitOS/output/linux-5.0-arm32/linux/linux/driver/BiscuitOS/Makefile
   
   modify Makefile:

   obj-$(CONFIG_PLATFORM_LDD_BASE) += platform.o
   ```

4. Configure Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32/linux/linux/
   make ARCH=arm memuconfig

   Device Driver --->
      BiscuitOS LDD --->
          <M> Platform LDD base
   ```
5. Compile Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32/linux/linux/
   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -j8
   ```

6. Running Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32
   ./RunBiscuitOs.sh
   ```

7. Review module on /proc

   ```
   Logon on BiscuitOS
   ~# cd /sys/devices/platform/Platform_demo
   ~# ls
   driver           modalias         subsystem
   driver_override  power            uevent
   ```

-----------------------------------------

#### <span id="B02">Build as module extral</span>

This platform device driver support build as module on extral
kernel source tree. follow steps:

1. Modify 'BSROOT' and 'CROSS_TOOLS' variable on Makefile. For example:

   ```
   The Project Path:
     -> BiscuitOS/output/linux-5.0-arm32/
   The kernel source tree Path:
     -> BiscuitOS/output/linux-5.0-arm32/linux/linux/
   And cross compile:
     -> arm-linux-gnueabi
   Current Architecture:
     -> ARM

   So, modify Makefile
     -> BSROOT=BiscuitOS/output/linux-5.0-arm32/
     -> CROSS_TOOLS=arm-linux-gnueabi
     -> ARCH=arm

     -> obj-m += platform.o
   ```

2. Compile module

   ```
   make
   ```
  
   And output information:

   ```
   make -C BiscuitOS/output/linux-5.0-arm32/linux/linux M=Platform/DTS/Base/kernel ARCH=arm \
                CROSS_COMPILE=BiscuitOS/output/linux-5.0-arm32/arm-linux-gnueabi/arm-linux-gnueabi/bin/arm-linux-gnueabi- modules
   make[1]: Entering directory 'BiscuitOS/output/linux-5.0-arm32/linux/linux-5.0'
     CC [M]  Platform/DTS/Base/kernel/platform.o
     Building modules, stage 2.
     MODPOST 1 modules
     CC      Platform/DTS/Base/kernel/platform.mod.o
     LD [M]  Platform/DTS/Base/kernel/platform.ko
     make[1]: Leaving directory 'BiscuitOS/output/linux-5.0-arm32/linux/linux-5.0'
   ```

3. Install module

   ```
   make install
   ```

   Install informaton:

   ```
   make -C BiscuitOS/output/linux-5.0-arm32/linux/linux M=Platform/DTS/Base/kernel ARCH=arm \
	INSTALL_MOD_PATH=BiscuitOS/output/linux-5.0-arm32/rootfs/rootfs/ modules_install
   make[1]: Entering directory 'BiscuitOS/output/linux-5.0-arm32/linux/linux-5.0'
   INSTALL Platform/DTS/Base/kernel/platform.ko
     DEPMOD  5.0.0
   make[1]: Leaving directory 'BiscuitOS/output/linux-5.0-arm32/linux/linux-5.0'
   ```

4. Refresh Rootfs

   ```
   cd BiscuitOS/output/linux-5.0-arm32/
   ./RunBiscuitOS.sh pack
   ```

5. Running Kernel

   ```
   cd BiscuitOS/output/linux-5.0-arm32
   ./RunBiscuitOs.sh
   ```

6. Insert module into kernel

   ```
   ~# cd /lib/modules/5.0.0/extra
   ~# insmod platform.ko
   ~# lsmod
   platform 16384 0 - Live 0x7f000000 (O)
   ~# modinfo platform.ko 
   filename:       platform.ko
   license:        GPL
   author:         BiscuitOS <buddy.zhang@aliyun.com>
   description:    Platform Device Driver with DTS
   alias:          of:N*T*CPlatform_demo,_BiscuitOSC*
   alias:          of:N*T*CPlatform_demo,_BiscuitOS
   depends:        
   vermagic:       5.0.0 SMP mod_unload ARMv7 p2v8 
   ```
   
7. Review module on /proc

   ```
   Logon on BiscuitOS
   ~# cd /sys/devices/platform/Platform_demo
   ~# ls
   driver           modalias         subsystem
   driver_override  power            uevent
   ```

8. Remove module

   ```
   ~# cd /lib/modules/5.0.0/extra
   ~# rmmod platform
   ```

--------------------------------------------

## <span id="A02">Userland</span>

When succeed installed an Platform device into system, we 
can access platform device via C interface or shell.

> - [C/C++ API](#C01)
>
> - [Shell](#C02)

--------------------------------

#### <span id="C01">C/C++ API</span>

On userspace, we can access platform device via C/C++ interface,
You can follow steps:

1. Modify 'ROOT', 'CROSS_NAME', and 'BSROOT' variables on Makefiel, 
   for example, We have a linux 5.0 project while path as follow:

   ```
   The 5.0 project path:
   -> BiscuitOS/output/linux-5.0-arm32/
   The BiscuitOS path:
   -> BiscuitOS/
   The cross-compiler:
   -> arm-linux-gnueabi

   So, we modify Makefile as follow:
   -> ROOT       := BiscuitOS/output/linux-5.0-arm32/
   -> BSROOT     := BiscuitOS/
   -> CROSS_NAME := arm-linux-gnueabi
   ```

2. Create directory on current directory, such as:

   ```
   mkdir -p App_demo-0.0.1
   cp platform.c App_demo-0.0.1
   ```

3. Compiler

   ```
   make
   ```

4. Install

   ```
   make install
   ```

5. Reference rootfs

   ```
   make pack
   ```

6. Running BiscuitOS

   ```
   cd BiscuitOS/output/linux-5.0-arm32/
   ./RunBiscuitOS
   ```

7. Running Application

   ```
   App_demo
   ```
  
   Output information:

   ```
   ~ # App_demo
   [LINK] subsystem
   [DIRT] power
   [FILE] driver_override
   [FILE] modalias
   [FILE] uevent
   [LINK] of_node

   ``` 

--------------------------------

#### <span id="C02"></span>

The shell scripts contains some useful command to operate platform
device. Detail see "userland/platform.sh"
