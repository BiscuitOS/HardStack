BiscuitOS DMABUF QEMU Module Usage
======================================

## Deploy

#### 1. Goto Qemu source code directory

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
```

**VERSION** is kernel version and **ARCH** is architecture of platformat. qemu-system is soft link from variable version qemu.

#### 2. Create BiscuitOS Device Module directory

Create BiscuitOS directory on **hw/**  directory, then modify Makefile.objs and Kconfig, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/
mkdir BiscuitOS
vi Makefile.objs

# Add context
devices-dirs-$(CONFIG_SOFTMMU) += BiscuitOS/

vi Kconfig

# Add context
source BiscuitOS/Kconfig
```

#### 3. Download module source code

We need download and copy source code from BiscuitOS, sach as:

```
cd BiscuitOS/
make linux-6.0-x86_64_defconfig
make menuconfig 

  [*] DIY BiscuitOS/Broiler Hardware  --->
      [*]   BiscuitOS PCI/PCIe DMA-BUF
  [*] Package --->
      [*] PCI: Peripheral Component Interconnect --->
          -*- QEMU PCI DEVICE: PCI DMA-BUF (BiscuitOS-DMA-BUF-*) --->
      [*] DMA: Coherent Memory Allocate --->
          [*] DMABUF: Video-Cpature with 2-GPUs --->

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-PCI-DMA-BUF-QEMU-DEVICE-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/BiscuitOS-PCI-DMA-BUF-QEMU-DEVICE-default ./ -rf
```

And the modify Makefile.objs and Kconfig under **hw/BiscuitOS/** directory, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
vi Makefile.objs

# Add context
common-obj-$(CONFIG_BISCUITOS_PCI_DMA_BUF) += BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/

vi Kconfig

# Add context
source BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/Kconfig
```

#### 4. Enable BiscuitOS Macro

We need create some macro to enable BiscuitOS module, **CONFIG_BISCUITOS_PCI_DMA_BUF** macro is used to enable BiscuitOS PCI module, **CONFIG_BISCUITOS_DMA** macro is used to enable BiscuitOS DMA module. We need enable those on special file, such as on X86 Emulator:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
vi default-configs/i386-softmmu.mak

# Add context
CONFIG_BISCUITOS_PCI_DMA_BUF=y

vi config-all-devices.mak

# Add context
CONFIG_BISCUITOS_PCI_DMA_BUF:=$(findstring y,$(CONFIG_BISCUITOS_PCI_DMA_BUF)y)
```

#### 5. Add QEMU command

The BiscuitOS PCI module is called from QEMU command, so we need add qemu options for module, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/
vi RunBiscuitOS.sh

# Check and if doesn't exist, then add context
        -hda ${ROOT}/BiscuitOS.img \
+       -device BiscuitOS-DMA-BUF-EXPORT \
+       -device BiscuitOS-DMA-BUF-IMPORTA \
+       -device BiscuitOS-DMA-BUF-IMPORTB \
        -drive file=${ROOT}/Freeze.img,if=virtio \
```

The name of PCI module is **BiscuitOS-DMA_BUF**, the options "-device" while invoke on QEMU.

#### 6. Compile QEMU

BiscuitOS provides a auto scripts to compile QEMU, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/
./RunQEMU.sh -b
```

The BiscuitOS is running when compile pass, and the BiscuitOS-pci module is running.

```
 ____  _                _ _    ___  ____  
| __ )(_)___  ___ _   _(_) |_ / _ \/ ___| 
|  _ \| / __|/ __| | | | | __| | | \___ \ 
| |_) | \__ \ (__| |_| | | |_| |_| |___) |
|____/|_|___/\___|\__,_|_|\__|\___/|____/ 
Welcome to BiscuitOS

Please press Enter to activate this console. 
/ # lspci | grep 1991
00:07.0 Class 00ff: 101a:1991
00:06.0 Class 00ff: 1019:1991
00:05.0 Class 00ff: 1018:1991
~ #
```

The ID 1018:1991 is BiscuitOS-DMA-BUF-EXPORT device, 1019:1991 is BiscuitOS-DMA-BUF-IMPORTA device, and 101a:1991 is BiscuitOS-DMA-BUF-IMPORTB device.

## File

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/BiscuitOS-DMA-DMABUF-with-QEMU-Device-default
tree

├── APP-Export-Capture.c
├── APP-Import-GPUA.c
├── APP-Import-GPUB.c
├── DMABUF-Export-Capture.c
├── DMABUF-Import-GPUA.c
├── DMABUF-Import-GPUB.c
├── Kconfig
├── Makefile
├── Makefile.host
└── RunBiscuitOS.sh
```
## Running Device Driver

The end, Running Device Driver on Guest, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/Broiler-dma-msix-default
make download
make build


# The BiscuitOS Running:
 ____  _                _ _    ___  ____
| __ )(_)___  ___ _   _(_) |_ / _ \/ ___|
|  _ \| / __|/ __| | | | | __| | | \___ \
| |_) | \__ \ (__| |_| | | |_| |_| |___) |
|____/|_|___/\___|\__,_|_|\__|\___/|____/
Welcome to BiscuitOS

Please press Enter to activate this console.
/ # RunBiscuitOS.sh 
[    7.267022] DMABUF_Export_Capture: loading out-of-tree module taints kernel.
[    7.269317] Default DMA Physical Address 0x25d8000
[    7.269320] BiscuitOS-DMABUF-Export  is Ready
[    7.271362] BiscuitOS-DMABUF-ImportA is Ready.
[    7.272935] BiscuitOS-DMABUF-ImportB is Ready.
[QEMU Video Capture] CA[0] Weclome to BiscuitOS Video Capture!
ReceiveA From Video: CA[0] Weclome to BiscuitOS Video Capture!
[QEMU GPUA NV 4090T] CA[0] Weclome to BiscuitOS Video Capture!
[    9.381425] GPU-A Receive Data From DDR
[QEMU Video Capture] CA[1] http://www.biscuitOS.cn
ReceiveB From Video: CA[1] http://www.biscuitOS.cn
[QEMU GPUB NV A100s] CA[1] http://www.biscuitOS.cn
[   12.387962] GPU-B Receive Data From DDR
/ #
```

## Blog

```
http://www.biscuitos.cn/blog/BiscuitOS_Catalogue/
```
