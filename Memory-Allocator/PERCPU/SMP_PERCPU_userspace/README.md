PERCPU(SMP) Memory Allocator on Userspace
--------------------------------------------

#### usage

```
make CPUS=8
./biscuitos
```

The `CPUS` determine the number of CPUs on Emualte platform.

------------------------------------------

#### standard output:

##### CPUs 2 Core

```
make CPUS=2
$ ./biscuitos 
BiscuitOS - PERCPU(UP) Memory Allocator
Physcial Memory Range: 0x60000000 - 0x61000000
Virtual Memory Ranges: 0xf6d91010 - 0xf7d91010
Embedded 7 pages/cpu @0xf7d7f010 s0 r8192 d20480 u32768
pcpu-alloc: s0 r8192 d20480 u32768 alloc=8*4096
pcpu-alloc: [0] 0 [0] 1 
CPU BiscuitOS-0
CPU BiscuitOS-1
bs[0]-cpu0: 0xf7d81010
bs[0]-cpu1: 0xf7d89010
bs[1]-cpu0: 0xf7d81030
bs[1]-cpu1: 0xf7d89030
bs[2]-cpu0: 0xf7d81050
bs[2]-cpu1: 0xf7d89050
bs[3]-cpu0: 0xf7d81070
bs[3]-cpu1: 0xf7d89070
bs[4]-cpu0: 0xf7d81090
bs[4]-cpu1: 0xf7d89090
bs[5]-cpu0: 0xf7d810b0
bs[5]-cpu1: 0xf7d890b0
bs[6]-cpu0: 0xf7d810d0
bs[6]-cpu1: 0xf7d890d0
bs[7]-cpu0: 0xf7d810f0
bs[7]-cpu1: 0xf7d890f0
bs[8]-cpu0: 0xf7d81110
bs[8]-cpu1: 0xf7d89110
bs[9]-cpu0: 0xf7d81130
bs[9]-cpu1: 0xf7d89130
```

##### CPUs 8 Core

```
make CPUS=8
$ ./biscuitos 
BiscuitOS - PERCPU(UP) Memory Allocator
Physcial Memory Range: 0x60000000 - 0x61000000
Virtual Memory Ranges: 0xf6cfb010 - 0xf7cfb010
Embedded 7 pages/cpu @0xf7cb9010 s0 r8192 d20480 u32768
pcpu-alloc: s0 r8192 d20480 u32768 alloc=8*4096
pcpu-alloc: [0] 0 [0] 1 [0] 2 [0] 3 [0] 4 [0] 5 [0] 6 [0] 7 
CPU BiscuitOS-0
CPU BiscuitOS-1
CPU BiscuitOS-2
CPU BiscuitOS-3
CPU BiscuitOS-4
CPU BiscuitOS-5
CPU BiscuitOS-6
CPU BiscuitOS-7
bs[0]-cpu0: 0xf7cbb010
bs[0]-cpu1: 0xf7cc3010
bs[0]-cpu2: 0xf7ccb010
bs[0]-cpu3: 0xf7cd3010
bs[0]-cpu4: 0xf7cdb010
bs[0]-cpu5: 0xf7ce3010
bs[0]-cpu6: 0xf7ceb010
bs[0]-cpu7: 0xf7cf3010
bs[1]-cpu0: 0xf7cbb030
bs[1]-cpu1: 0xf7cc3030
bs[1]-cpu2: 0xf7ccb030
bs[1]-cpu3: 0xf7cd3030
bs[1]-cpu4: 0xf7cdb030
bs[1]-cpu5: 0xf7ce3030
bs[1]-cpu6: 0xf7ceb030
bs[1]-cpu7: 0xf7cf3030
bs[2]-cpu0: 0xf7cbb050
bs[2]-cpu1: 0xf7cc3050
bs[2]-cpu2: 0xf7ccb050
bs[2]-cpu3: 0xf7cd3050
bs[2]-cpu4: 0xf7cdb050
bs[2]-cpu5: 0xf7ce3050
bs[2]-cpu6: 0xf7ceb050
bs[2]-cpu7: 0xf7cf3050
bs[3]-cpu0: 0xf7cbb070
bs[3]-cpu1: 0xf7cc3070
bs[3]-cpu2: 0xf7ccb070
bs[3]-cpu3: 0xf7cd3070
bs[3]-cpu4: 0xf7cdb070
bs[3]-cpu5: 0xf7ce3070
bs[3]-cpu6: 0xf7ceb070
bs[3]-cpu7: 0xf7cf3070
bs[4]-cpu0: 0xf7cbb090
bs[4]-cpu1: 0xf7cc3090
bs[4]-cpu2: 0xf7ccb090
bs[4]-cpu3: 0xf7cd3090
bs[4]-cpu4: 0xf7cdb090
bs[4]-cpu5: 0xf7ce3090
bs[4]-cpu6: 0xf7ceb090
bs[4]-cpu7: 0xf7cf3090
bs[5]-cpu0: 0xf7cbb0b0
bs[5]-cpu1: 0xf7cc30b0
bs[5]-cpu2: 0xf7ccb0b0
bs[5]-cpu3: 0xf7cd30b0
bs[5]-cpu4: 0xf7cdb0b0
bs[5]-cpu5: 0xf7ce30b0
bs[5]-cpu6: 0xf7ceb0b0
bs[5]-cpu7: 0xf7cf30b0
bs[6]-cpu0: 0xf7cbb0d0
bs[6]-cpu1: 0xf7cc30d0
bs[6]-cpu2: 0xf7ccb0d0
bs[6]-cpu3: 0xf7cd30d0
bs[6]-cpu4: 0xf7cdb0d0
bs[6]-cpu5: 0xf7ce30d0
bs[6]-cpu6: 0xf7ceb0d0
bs[6]-cpu7: 0xf7cf30d0
bs[7]-cpu0: 0xf7cbb0f0
bs[7]-cpu1: 0xf7cc30f0
bs[7]-cpu2: 0xf7ccb0f0
bs[7]-cpu3: 0xf7cd30f0
bs[7]-cpu4: 0xf7cdb0f0
bs[7]-cpu5: 0xf7ce30f0
bs[7]-cpu6: 0xf7ceb0f0
bs[7]-cpu7: 0xf7cf30f0
bs[8]-cpu0: 0xf7cbb110
bs[8]-cpu1: 0xf7cc3110
bs[8]-cpu2: 0xf7ccb110
bs[8]-cpu3: 0xf7cd3110
bs[8]-cpu4: 0xf7cdb110
bs[8]-cpu5: 0xf7ce3110
bs[8]-cpu6: 0xf7ceb110
bs[8]-cpu7: 0xf7cf3110
bs[9]-cpu0: 0xf7cbb130
bs[9]-cpu1: 0xf7cc3130
bs[9]-cpu2: 0xf7ccb130
bs[9]-cpu3: 0xf7cd3130
bs[9]-cpu4: 0xf7cdb130
bs[9]-cpu5: 0xf7ce3130
bs[9]-cpu6: 0xf7ceb130
bs[9]-cpu7: 0xf7cf3130
```

#### Logic Diagram

![](https://gitee.com/BiscuitOS_team/PictureSet/raw/Gitee/HK/HK000223.png)

![](https://gitee.com/BiscuitOS_team/PictureSet/raw/Gitee/HK/HK000224.png)

