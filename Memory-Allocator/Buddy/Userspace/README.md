Buddy Memory Allocator on Userspace
--------------------------------------------

#### usage

```
make clean
make
./biscuitos
```

#### Output information

```
$ ./biscuitos 
BiscuitOS Buddy Memory Allocator.
Physical Memory: 0x60000000 - 0x61000000
mem_map[] contains 0x1000 pages, page size 0x1000
16-Pages Phys:  0x6000c000 PFN: 0x60c00
16-Pages Phys:  0x6000c000 PFN: 0x60c00
128-Pages Phys: 0x6000c800 PFN: 0x60c80
Freeing 16-pages.
Freeing 128-pages.
Page-PFN: 0x60c00 Address 0xf7970010
Buddy Output: BiscuitOs-88
```
