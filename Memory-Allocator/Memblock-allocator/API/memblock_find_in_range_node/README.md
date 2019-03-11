memblock_find_in_range_node [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-memblock_find_in_range_node/)
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
            [*]memblock_find_in_range_node()
```
