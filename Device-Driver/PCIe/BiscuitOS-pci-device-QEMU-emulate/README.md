BiscuitOS PCI QEMU Module Usage
======================================

1. Goto Qemu source code directory

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
```

**VERSION** is kernel version and **ARCH** is architecture of platformat. qemu-system is soft link from variable version qemu.

2. Create BiscuitOS PCI device directory

Create BiscuitOS directory on **hw/**  directory, and modify Makefile.objs, such as:

```
cd hw/
mkdir BiscuitOS
vi Makefile.objs
devices-dirs-$(CONFIG_SOFTMMU) += BiscuitOS/
```

And modify Kconfig on **hw/**, such as:

```
source BiscuitOS/Kconfig
```

3. Download module source code

We need download and copy source code from BiscuitOS, sach as:

```
cd BiscuitOS/
make linux-5.0-x86_64_defconfig
make menuconfig 

  [*] Package --->
      [*] PCI: Peripheral Component Interconnect --->
          [*] PCI Common Device Driver Module --->
          [*] QEMU emulate PCIe Device (BiscuitOS-pcie) --->

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-pci-device-QEMU-emulate-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-pci-device-QEMU-emulate-default/BiscuitOS-pci-device-QEMU-emulate-default ./ -rf
```

And the modify Makefile.objs and Kconfig under **hw/BiscuitOS/** directory, such as:

```
vi Makefile.objs

common-obj-$(CONFIG_BISCUITOS_PCI) += BiscuitOS-pci-device-QEMU-emulate-default/

vi Kconfig

source BiscuitOS-pci-device-QEMU-emulate-default/Kconfig
```

5. Enable BiscuitOS Macro

We need create some macro to enable BiscuitOS module, **CONFIG_BISCUITOS_PCI** macro is used to enable BiscuitOS PCI module, **CONFIG_BISCUITOS_DMA** macro is used to enable BiscuitOS DMA module. We need enable those on special file, such as on X86 Emulator:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
vi default-configs/i386-softmmu.mak

# Add context
CONFIG_BISCUITOS_PCI=y
```

6. Add QEMU command

The BiscuitOS PCI module is called from QEMU command, so we need add qemu options for module, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/
vi RunBiscuitOS.sh

# Add context
        -hda ${ROOT}/BiscuitOS.img \
+       -device BiscuitOS-pci \
        -drive file=${ROOT}/Freeze.img,if=virtio \
```

7. Compile QEMU

BiscuitOS provides a auto scripts to compile QEMU, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/
./RunQEMU.sh -b
```

The BiscuitOS is running when compile pass, and the BiscuitOS-pci module is running.
