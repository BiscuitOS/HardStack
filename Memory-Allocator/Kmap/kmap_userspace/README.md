FIXMAP/KMAP Memory Allocator on Userspace
------------------------------

#### Usage

```
make
./biscuitos
```

#### Standard output information

```
$ ./biscuitos 
BiscuitOS High-Memory
Real Physical Memory:  0x60000000 - 0x61200000
Normal Physical Areas: 0x60000000 - 0x61000000
HighMem Physical Area: 0x61000000 - 0x61200000
Virtual Memory:        0xf6b9b010 - 0xf7b9b010
mem_map[] contains 0x1200 pages, page size 0x1000
SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1
PAGE-Table-Directory: 0xf779f010 - 0xf77a3010
KMAP Page-Direct:     0xf779f010 - 0xf77a1008
KMAP AREA:    0x7fe00000 - 0x80000000
FIXMAP AREA:  0xffc00000 - 0xfff00000
FIXMAP-ADDR:  0xffeee000
KMAP-Atomic:  BiscuitOS-89
FIXMAP Addr:  0xffeee000
FIXMAP Addr:  0xffeed000
FIXMAP Addr:  0xffeec000
FIXMAP Addr:  0xffeeb000
FIXMAP Addr:  0xffeea000
FIXMAP Addr:  0xffee9000
FIXMAP Addr:  0xffee8000
FIXMAP Addr:  0xffee7000
FIXMAP Addr:  0xffee6000
FIXMAP Addr:  0xffee5000
HighMem-PFN:  0x61000
KMAP Address: 0x7fe01000
Output Info:  BiscuitOs-93
Output Info:  BiscuitOS-96
KMAP Address: 0x7fe01000
KMAP Address: 0x7fe02000
KMAP Address: 0x7fe03000
KMAP Address: 0x7fe04000
KMAP Address: 0x7fe05000
KMAP Address: 0x7fe06000
KMAP Address: 0x7fe07000
KMAP Address: 0x7fe08000
KMAP Address: 0x7fe09000
KMAP Address: 0x7fe0a000
```
