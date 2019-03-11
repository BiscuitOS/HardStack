memblock_remove [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-memblock_remove/)
--------------------------------------------

* function: Remove special memory region from memblock.memory.


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
            [*]memblock_remove()
```
