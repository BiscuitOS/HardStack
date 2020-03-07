BiscuitOS tmpfs Filesystem
------------------------------------------

> - [1. Common](#C)
>
> - [2. ftrace](#A)
>
> - [3. tmpfs_bs usage](#B)
>
> - [4. Trace tmpfs_bs](#D)

-------------------------------------------

## <span id="C">1. Common</span>

The "tmpfs_bs" is a simple RAMFS and copies from linux tmpfs, it is
used to individually mount and tracing function invoke and called,
it's useful to help someone to understand and practice principle for
"tmpfs". Here, I offer some tools and mind to achieve all, U can
do as follow segment.

------------------------------------

## <span id="A">2. ftrace</span>

`ftrace` is used to tracing special function invoke and called routine.
Here, we use some function of `ftrace` to tracing tmpfs_bs function.
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

## <span id="B">3. tmpfs_bs usage</span>

If you want to use "tmpfs_bs" on BiscuitOS, it's easy as follow:

#### 3.1 mount

```
~ # mkdir -p /tmpfs_bs
~ # mount -t tmpfs_bs tmpfs_BiscuitOS /tmpfs_bs
~ # mount
/dev/root on / type ext4 (rw,relatime)
proc on /proc type proc (rw,relatime)
tmpfs on /tmp type tmpfs (rw,relatime)
sysfs on /sys type sysfs (rw,relatime)
tmpfs on /dev type tmpfs (rw,relatime)
debugfs on /sys/kernel/debug type debugfs (rw,relatime)
devpts on /dev/pts type devpts (rw,relatime,mode=600,ptmxmode=000)
/dev/vdb on /mnt/Freeze type ext4 (rw,relatime)
tmpfs_BiscuitOS on /tmpfs_bs type tmpfs_bs (rw,relatime,mode=0)
~ # 
```

#### 3.2 Setup Ramfs size

```
~ # mkdir -p /tmpfs_bs
~ # mount -t tmpfs_bs -o size=8M tmpfs_BiscuitOS /tmpfs_bs
```

--------------------------------------

## <span id="D">4. Trace tmpfs_bs</span>

Here I will describe how to trace all routine for "tmpfs_bs", As we known,
Usespace often requests system service by system-call, the OS will deal
with and ACK. So at first, we need to track on usespace, we can do as
follow:

#### 4.1 mount tmpfs_bs

```
~ # strace mount -t tmpfs_bs tmpfs_BiscuitOS /tmpfs_bs
```

The `strace` tools is used to tracing system-call on userspace, after
executed above command, the terminal will output information:

```
execve("/bin/mount", ["mount", "-t", "tmpfs_bs", "tmpfs_BiscuitOS", "/tmpfs_bs"], 0x7ec29e50 /* 9 vars */) = 0
brk(NULL)                               = 0x181000
brk(0x181d24)                           = 0x181d24
set_tls(0x1814c0)                       = 0
uname({sysname="Linux", nodename="BiscuitOS", ...}) = 0
readlink("/proc/self/exe", "/bin/busybox", 4096) = 12
brk(0x1a2d24)                           = 0x1a2d24
brk(0x1a3000)                           = 0x1a3000
getuid32()                              = 0
getuid32()                              = 0
geteuid32()                             = 0
stat64("tmpfs_BiscuitOS", 0x7e80eb80)   = -1 ENOENT (No such file or directory)
mount("tmpfs_BiscuitOS", "/tmpfs_bs", "tmpfs_bs", MS_SILENT, NULL) = 0
exit_group(0)                           = ?
+++ exited with 0 +++
~ # 

```

Some no userful information we can filter, and "mount(...)" function is 
important to mount "tmpfs_bs" on system, and "mount(...)" will trigger 
the system-call "sys_mount" which defined by Architecture. On `ARM`,
we can find entry for "sys_mount" as follow:

```
arch/arm/include/generated/calls-eabi.S
arch/arm/include/generated/calls-oabi.S

Define:

NATIVE(21, sys_mount)
```  

The Architecture only define a entry to sys_mount, and this entry will
invoke the common entry on:

```
fs/namespace.c

SYSCALL_DEFINE5(mount, char __user *, dev_name, char __user *, dir_name,
                char __user *, type, unsigned long, flags, void __user *, data)
{
        return ksys_mount(dev_name, dir_name, type, flags, data);
}
```

-----------------------------------

#### 4.2 cd command

After tmpfs_bs mounted, we use `cd` change current directory into top
direct for "tmpfs_bs", in order to trancing it, do as follow:

```
~ # RunBiscuitOS_ftrace.sh init
~ # cd /tmpfs_bs
~ # RunBiscuitOS_ftrace.sh show
```

When above command executed, the ftrace will show all function which tmpfs_bs
filesystem called, like:

```
# tracer: function
#
# entries-in-buffer/entries-written: 0/0   #P:1
#
#                              _-----=> irqs-off
#                             / _----=> need-resched
#                            | / _---=> hardirq/softirq
#                            || / _--=> preempt-depth
#                            ||| /     delay
#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION
#              | |       |   ||||       |         |
/tmpfs_bs # 
/tmpfs_bs # 
```

Now, no function be called from tmpfs_bs filesytem, but if we want to 
know how system execute `cd /tmpfs_bs`, we can trace from userspace,
like:

```
~ # strace cd /tmpfs_bs
```

-------------------------------------

#### 4.3 create directory

On "tmpfs_bs", we can create a new directory and trace it, as follow:

```
~ # RunBiscuitOS_ftrace.sh init
~ # cd /tmpfs_bs
~ # mkdir bus
~ # RunBiscuitOS_ftrace.sh show
```

When above command executed, the ftrace will show all function which
"tmpfs_bs" filesystem called, detail as follow:

```
/tmpfs_bs # RunBiscuitOS_ftrace.sh show
# tracer: function
#
# entries-in-buffer/entries-written: 4/4   #P:1
#
#                              _-----=> irqs-off
#                             / _----=> need-resched
#                            | / _---=> hardirq/softirq
#                            || / _--=> preempt-depth
#                            ||| /     delay
#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION
#              | |       |   ||||       |         |
           mkdir-874   [000] .n..   111.475146: shmem_mkdir_bs <-vfs_mkdir
           mkdir-874   [000] .n..   111.475894: shmem_mknod_bs <-shmem_mkdir_bs
           mkdir-874   [000] .n..   111.475956: shmem_get_inode_bs <-shmem_mknod_bs
           mkdir-874   [000] .n..   111.475975: shmem_alloc_inode_bs <-alloc_inode
/tmpfs_bs #
```

And "vfs_mkdir" is main entry for `mkdir bus` on "tmpfs_bs", we can use
`strace` to tracing `mkdir` on userspace, as follow:

```
~ # cd /tmpfs_bs
~ # strace mkdir BiscuitOS_bus
execve("/bin/mkdir", ["mkdir", "BiscuitOS_bus"], 0x7e991e64 /* 10 vars */) = 0
brk(NULL)                               = 0x181000
brk(0x181d24)                           = 0x181d24
set_tls(0x1814c0)                       = 0
uname({sysname="Linux", nodename="BiscuitOS", ...}) = 0
readlink("/proc/self/exe", "/bin/busybox", 4096) = 12
brk(0x1a2d24)                           = 0x1a2d24
brk(0x1a3000)                           = 0x1a3000
getuid32()                              = 0
mkdir("BiscuitOS_bus", 0777)            = 0
exit_group(0)                           = ?
+++ exited with 0 +++
/tmpfs_bs #
/tmpfs_bs #
```

On usespace, `mkdir()` command is end function for calling, it will invoke
system call `sys_mkdir`, and skip Architecture, the `sys_mkdir` will invoke
`do_mkdirat`, detail as follow:

```
fs/namei.c

SYSCALL_DEFINE2(mkdir, const char __user *, pathname, umode_t, mode)
{
        return do_mkdirat(AT_FDCWD, pathname, mode);
}
```
























