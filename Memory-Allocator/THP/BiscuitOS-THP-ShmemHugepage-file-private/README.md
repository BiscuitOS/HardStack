THP: ShmemHugepage on File
===============================

#### Prepare

we need enable macro "CONFIG_TRANSPARENT_HUGEPAGE" and "CONFIG_TRANSPARENT_HUGEPAGE_ALWAYS", and total RAM is must bigger then 512MiB!

Then, we need enable shmem_huge, e.g.

```
mkdir -p /BiscuitOS-tmpfs/
mount -t tmpfs nodev -o huge=always /BiscuitOS-tmpfs/
```

Then cat /proc/meminfo

```
AnonHugePages:         0 kB
ShmemHugePages:     2048 kB
ShmemPmdMapped:        0 kB
CmaTotal:              0 kB
```
