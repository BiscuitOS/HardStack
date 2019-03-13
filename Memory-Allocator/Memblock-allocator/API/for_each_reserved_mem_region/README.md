for_each_reserved_mem_region [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-for_each_reserved_mem_region/)
--------------------------------------------

* function: Free an reserved region.


## Usage

* patch file

```
git am *.patch kernel_src/
```

* driver file

```
Device Driver--->
    [*]BiscuitOS Driver--->
        [*]Memblock allocator
            [*]for_each_reserved_mem_region()
```
