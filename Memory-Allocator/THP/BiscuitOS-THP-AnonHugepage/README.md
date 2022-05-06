THP: AnonHugepage
===============================

#### Prepare

we need enable macro "CONFIG_TRANSPARENT_HUGEPAGE" and "CONFIG_TRANSPARENT_HUGEPAGE_ALWAYS", and total RAM is must bigger then 512MiB!

Then, we need enable THP always, e.g.

```
echo always > /sys/kernel/mm/transparent_hugepage/enabled
```

Then cat /proc/meminfo

```
AnonHugePages:      8192 kB
ShmemHugePages:        0 kB
ShmemPmdMapped:        0 kB
CmaTotal:              0 kB
```
