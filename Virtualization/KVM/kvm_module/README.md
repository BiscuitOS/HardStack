KVM on Kernel
---------------------------------------

> - [Kernel Configure](#A0)

---------------------------

#### <span id="A0">Kernel Configure</span>

###### i386

```
make menuconfig ARCH=i386

  [*] Virtualization --->
      <M>   Kernel-based Virtual Machine (KVM) support
      <M>     KVM for Intel processors support
      <M>     KVM for AMD processors support
      <M>   Host kernel accelerator for virtio net
      [*]   Cross-endian support for vhost

  Processor type and features  --->
      [*] Linux guest support  --->
```
