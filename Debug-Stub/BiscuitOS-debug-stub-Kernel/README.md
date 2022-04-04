BiscuitOS Kernel Debug Stub Usage
======================================

## Deploy

#### 1. Goto Qemu source code directory

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
```

**VERSION** is kernel version and **ARCH** is architecture of platformat. qemu-system is soft link from variable version qemu.

#### 2. Download Stub source code

We need download and copy source code from BiscuitOS, sach as:

```
cd BiscuitOS/
make linux-5.0-x86_64_defconfig
make menuconfig 

  [*] Package --->
      [*] BiscuitOS Debug Stub Set --->
          [*] BiscuitOS Kernel Debug Stub --->

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-Kernel-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/linux/linux
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-Kernel-default/BiscuitOS-debug-stub-Kernel-default/BiscuitOS-stub.c ./lib/ -rf
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-debug-stub-Kernel-default/BiscuitOS-debug-stub-Kernel-default/BiscuitOS-stub.h ./include/linux/ -rf
```

And the modify **lib/Makefile** and **include/linux/kernel.h** under current directory, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/linux/linux
vi lib/Makefile

# Add context
obj-y += BiscuitOS-stub.o

cd BiscuitOS/output/linux-${VERSION}-${ARCH}/linux/linux/include/
vi include/linux/kernel.h

# Add context
+ #include "BiscuitOS-stub.h"
```

#### 3. Deploy syscall

On different architercture need different way to deploy syscall, such as:

###### X86/AMD64

```
cd BiscuitOS/output/linux-${VERSION}-x86/linux/linux
vi arch/x86/entry/syscalls/syscall_64.tbl

# Add context on bottom
+ 600     common  debug_BiscuitOS         __x64_sys_debug_BiscuitOS
```

###### i386

```
cd BiscuitOS/output/linux-${VERSION}-i386/linux/linux
vi arch/x86/entry/syscalls/syscall_32.tbl

# Add context on bottom
+ 387     i386  debug_BiscuitOS     sys_debug_BiscuitOS    __ia32_sys_debug_BiscuitOS
```

###### ARM32

```
cd BiscuitOS/output/linux-${VERSION}-arm/linux/linux
vi arch/arm/tools/syscall.tbl

# Add context on bottom
+ 400    common  debug_BiscuitOS     sys_debug_BiscuitOS
```

###### ARM64

```
cd BiscuitOS/output/linux-${VERSION}-aarch/linux/linux
vi include/uapi/asm-generic/unistd.h

# Add context on bottom
+ define __NR_debug_BiscuitOS 400
+ __SYSCALL(__NR_debug_BiscuitOS, sys_debug_BiscuitOS)

- #define __NR_syscalls 400
+ #define __NR_syscalls 401
```

On different, we need special syscall number.

#### 4. Compile Kernel

BiscuitOS provides a auto scripts to compile Kernel, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-kernel-default/
make build
```

4. usage

On C/C++ Userpace program, we need SYSCALL to disable or enable kernel stub, on different use different SYSCALL number, the value of BISCUITOS_SYSCALL is **x86:400**/**i386:387**/**arm32:400**/**arm64:400**:

```
#include <stdio.h>
#include <sys/types.h
#include <unistd.h>

int main(void)
{
	...
	/* Enable Kernel Stub */
	syscall(BISCUITOS_SYSCALL_NR, 1);

	/* syscall */
	...

	/* Disable Kernel Stub */
	syscall(BISCUITOS_SYSCALL_NR, 0);
	...
}

```

we can use **strace** to trace system call information. If we want to debug scripts, we can use **/proc/sys/BiscuitOS/bs_debug_enable**, write 1 to enable kernel stub, and 0 to disable kernel stub, such as:

```
# Example shell scripts

#!/bin/bash

...

/* Enable kernel Stub */
echo 1 > /proc/sys/BiscuitOS/bs_debug_enable

...

/* Disable kernel Stub */
echo 0 > /proc/sys/BiscuitOS/bs_debug_enable

...

```

Then on kernel, we can use **bs_debug()** to print special information, and use **bs_kdebug()** and dmesg to obtain speical information.

#### Blog

```
https://biscuitos.github.io/blog/BiscuitOS_Catalogue/
```
