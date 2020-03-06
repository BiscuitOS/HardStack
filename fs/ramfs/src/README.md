BiscuitOS ramfs Filesystem
------------------------------------------

> - [1. Common](#C)
>
> - [2. ftrace](#A)
>
> - [3. ramfs_bs usage](#B)

-------------------------------------------

## <span id="C">1. Common</span>

The "ramfs_bs" is a simple RAMFS and copies from linux ramfs, it is
used to individually mount and tracing function invoke and called,
it's useful to help someone to understand and practice principle for
"ramfs". Here, I offer some tools and mind to achieve all, U can
do as follow segment.

------------------------------------

## <span id="A">2. ftrace</span>

`ftrace` is used to tracing special function invoke and called routine.
Here, we use some function of `ftrace` to tracing ramfs_bs function.
Buddy, you can do as follow:

#### 2.1 init

Before use ftrace, we need open some Kconfig Config macro, like:

```
make menuconfig ARCH=arm

Kernel hacking  --->
   [*] Tracers  --->
      [*]   Kernel Function Tracer
      [*]   enable/disable function tracing dynamically (NEW)
```

Save and rebuild kernel.

#### 2.2 install

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

#### 2.3 usage

After installed, we can use `RunBiscuitOS_ftrace.sh` to trace proc_bs
function, detail as follow:

##### 2.3.1 Init

Before using, we need init my ftrace, as follow:

```
~ # RunBiscuitOS_ftrace.sh init
```

##### 2.3.2 Show context

When tracing, we can capture and dump all trace information as follow:

```
~ # RunBiscuitOS_ftrace.sh show
```

##### 2.3.3 Clear buffer

When I want clear capture buffer, we can do that:

```
~ # RunBiscuitOS_ftrace.sh clear
```

------------------------------------

## <span id="B">3. ramfs_bs usage</span>

If you want to use "ramfs_bs" on BiscuitOS, it's easy as follow:

#### 3.1 mount

```
~ # mkdir -p /ramfs_bs
~ # mount -t ramfs_bs ramfs_BiscuitOS /ramfs_bs
~ # mount
/dev/root on / type ext4 (rw,relatime)
proc on /proc type proc (rw,relatime)
tmpfs on /tmp type tmpfs (rw,relatime)
sysfs on /sys type sysfs (rw,relatime)
tmpfs on /dev type tmpfs (rw,relatime)
debugfs on /sys/kernel/debug type debugfs (rw,relatime)
devpts on /dev/pts type devpts (rw,relatime,mode=600,ptmxmode=000)
/dev/vdb on /mnt/Freeze type ext4 (rw,relatime)
ramfs_BiscuitOS on /ramfs_bs type ramfs_bs (rw,relatime,mode=0)
~ # 
```

#### 3.2 Setup Ramfs size

```
~ # mkdir -p /ramfs_bs
~ # mount -t ramfs_bs -o size=8M ramfs_BiscuitOS /ramfs_bs
```
