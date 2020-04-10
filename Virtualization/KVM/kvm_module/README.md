KVM on Kernel
---------------------------------------

> - [Kernel Configure](#A0)
>
> - [Module usage](#B0)
>
> - [ftrace](#C0)

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


  Kernel hacking  --->
      [*] Tracers  --->
         [*]   Kernel Function Tracer
         [*]   enable/disable function tracing dynamically (NEW)
```


-------------------------------------

#### <span id="B0">Module Usage</span>

```
RunBiscuitOS_kvm.sh mount
```

--------------------------------------

#### <span id="C0">ftrace</span>

```
RunBiscuitOS_kvm.sh init
RunBiscuitOS_kvm.sh on
RunBiscuitOS_kvm.sh show
```


