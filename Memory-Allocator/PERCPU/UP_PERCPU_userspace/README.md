PERCPU(UP) Memory Allocator on Userspace
--------------------------------------------

##### usage

```
make
./biscuitos
```

##### standard output:

```
$ ./biscuitos 
BiscuitOS - PERCPU(UP) Memory Allocator
Physcial Memory Range: 0x60000000 - 0x61000000
Virtual Memory Ranges: 0xf6db9010 - 0xf7db9010
pcpu-alloc: s0 r0 d32768 u32768 alloc=1*32768
pcpu-alloc: [0] 0 
UP-PERCPU Allocator Initialized finished.
CPU BiscuitOS-0
```

#### Logic Diagram

![](https://gitee.com/BiscuitOS_team/PictureSet/raw/Gitee/HK/HK000223.png)

![](https://gitee.com/BiscuitOS_team/PictureSet/raw/Gitee/HK/HK000223.png)

