memblock_free [中文教程](https://biscuitos.github.io/blog/MMU-ARM32-MEMBLOCK-memblock_free/)
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
            [*]memblock_free()
```
