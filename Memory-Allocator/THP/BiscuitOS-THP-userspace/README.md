THP on userpsace
===============================

#### Prepare

we need enable macro "CONFIG_TRANSPARENT_HUGEPAGE" and "CONFIG_TRANSPARENT_HUGEPAGE_ALWAYS", and total RAM is must bigger then 512MiB!

Then, we need enable shmem_huge, e.g.

```
echo always > /sys/kernel/mm/transparent_hugepage/shmem_enabled
``
