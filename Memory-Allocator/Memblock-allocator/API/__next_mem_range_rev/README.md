__next_mem_range_rev [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-__next_mem_range_rev/)
--------------------------------------------

* function: generia next function for for_each_*_range_rev


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
            [*]__next_mem_range_rev()
```
