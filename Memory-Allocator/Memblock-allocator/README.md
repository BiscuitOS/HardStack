MEMBLOCK Allocator [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-index/)
--------------------------------------------

## Overview

Memblock is a method of managing memory regions during the early
boot period when the usual kernel memory allocators are not up and
running.

Memblock views the system memory as collections of contiguous
regions. There are several types of these collections:

* memory
 
  describes the physical memory available to the kernel; this 
  may differ from the actual physical memory installed in the 
  system, for instance when the memory is restricted with `mem=` 
  command line parameter

* reserved

  describes the regions that were allocated
 
* physmap

  describes the actual physical memory regardless of the possible
  restrictions; the ``physmap`` type is only available on some 
  architectures.

## Figure

```
MEMBLOCK


                                         struct memblock_region
                       struct            +------+------+--------+------+
                       memblock_type     |      |      |        |      |
                       +----------+      | Reg0 | Reg1 | ...    | Regn |
                       |          |      |      |      |        |      |
                       | regions -|----->+------+------+--------+------+
                       | cnt      |      [memblock_memory_init_regions]
                       |          |
 struct           o--->+----------+
 memblock         |
 +-----------+    |
 |           |    |
 | memory   -|----o
 | reserved -|----o
 |           |    |                      struct memblock_region
 +-----------+    |    struct            +------+------+--------+------+
                  |    memblock_type     |      |      |        |      |
                  o--->+----------+      | Reg0 | Reg1 | ...    | Regn |
                       |          |      |      |      |        |      |
                       | regions -|----->+------+------+--------+------+
                       | cnt      |      [memblock_reserved_init_regions]
                       |          |
                       +----------+
```

## Dir

* API

  MEMBLOCK Application Interface on Linux Kernel.
