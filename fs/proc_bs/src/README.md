BiscuitOS Proc Filesystem
------------------------------------------

> - [ftrace](#A)
>
> - [proc_bs usage](#B)

------------------------------------

## <span id="A">ftrace</span>

#### init

Before use ftrace, we need open some Kconfig Config macro, like:

```
make menuconfig ARCH=arm

Kernel hacking  --->
   [*] Tracers  --->
      [*]   Kernel Function Tracer
      [*]   enable/disable function tracing dynamically (NEW)
```

Save and rebuild kernel.

#### install

The scripts `RunBiscuitOS_ftrace.sh` is used to simply use ftrace to
trace proc_bs filesystem. At first, we should install scripts onto 
BiscuitOs, as follow:

```
cd BiscuitOS/output/linux-5.0-arm32/
./RunBiscuitOS.sh mount
cp RunBiscuitOS_ftrace.sh BiscuitOS/output/linux-5.0-arm32/FreezeDir
./RunBiscuitOS.sh umount
./RunBiscuitOS.sh

~ # cp /mnt/Freeze/RunBiscuitOS_ftrace.sh /usr/bin
~ # chmod 755 /usr/bin/RunBiscuitOS_ftrace.sh
```

#### usage

After installed, we can use `RunBiscuitOS_ftrace.sh` to trace proc_bs
function, detail as follow:

##### Init

Before using, we need init my ftrace, as follow:

```
~ # RunBiscuitOS_ftrace.sh init
```

##### Show context

When tracing, we can capture and dump all trace information as follow:

```
~ # RunBiscuitOS_ftrace.sh show
```

##### Clear buffer

When I want clear capture buffer, we can do that:

```
~ # RunBiscuitOS_ftrace.sh clear
```

------------------------------------

## <span id="B">proc_bs usage</span>

#### mount

```
mkdir -p /proc_bs
mount -t proc_bs nodev /proc_bs
```
