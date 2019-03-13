memblock_alloc_base_nid [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-memblock_alloc_base_nid/)
--------------------------------------------

* function: Find a valid memory from special area.


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
            [*]memblock_alloc_base_nid()
```
