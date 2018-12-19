SPI on uboot
-----------------------------------

On Soc, some system boot from SPI-flash, and developer need do read- or write-
operation for SPI-flash on uboot. The common distro uboot offers `sf` toolchain
to read, write, or init SPI-flash. The follow contents is used to describe
how to use `sf` easily.

# sf -- SPI flash sub-system

Usermanual of 'sf'

```
ZynqMP> sf
sf - SPI flash sub-system

Usage:
sf probe [[bus:]cs] [hz] [mode] - init flash device on given SPI bus
                                  and chip select
sf read addr offset|partition len       - read `len' bytes starting at
                                          `offset' or from start of mtd
                                          `partition'to memory at `addr'
sf write addr offset|partition len      - write `len' bytes from memory
                                          at `addr' to flash at `offset'
                                          or to start of mtd `partition'
sf erase offset|partition [+]len        - erase `len' bytes from `offset'
                                          or from start of mtd `partition'
                                         `+len' round up `len' to block size
sf update addr offset|partition len     - erase and write `len' bytes from memoy
                                          at `addr' to flash at `offset'
                                          or to start of mtd `partition'
sf protect lock/unlock sector len       - protect/unprotect 'len' bytes starting
                                          at address 'sector'

ZynqMP> 
```

### Init SPI-flash

`sf probe` is used to initialize SPI-flash device on given SPI bus and chip 
select.

Usage:

```
sf probe [[bus:]cs] [hz] [mode]
```

e.g. Detect current valid SPI-flash.

```
ZynqMP> sf probe
SF: Detected n25q256a with page size 512 Bytes, erase size 128 KiB, total 64 MiB
ZynqMP> 
```

### Read 

`sf read`: read `len` bytes starting at `offset` or from start of mtd 
`partition` to memory at `addr`.

Usage:

```
sf read addr offset|partition len
```

e.g. Read 0x2000 bytes starting at 0x100 to memory at 0x40000.

```
ZynqMP> sf read 0x40000 0x100 0x2000
device 0 offset 0x100, size 0x2000
SF: 8192 bytes @ 0x100 Read: OK
ZynqMP>
```

### Write

`sf write`: write `len` bytes from memory at `addr` to flash at `offset` or
to start of mtd `partition`.

Usage:

```
sf write addr offset|partition len
```

e.g. Write 0x10 bytes from memory at 0x40000 to flash at 0x3F00000.

```
ZynqMP> sf write 0x40000 0x3F00000 0x10
device 0 offset 0x3f00000, size 0x10
SF: 16 bytes @ 0x3f00000 Written: OK
ZynqMP>
```

### Erase SPI-flash

Erase `len` bytes from `offset` or from start of mtd `partition`

e.g. Erase 0x100000 bytes from 0x3F00000.

```
ZynqMP> sf erase 0x3f00000 0x100000
SF: 1048576 bytes @ 0x3f00000 Erased: OK
ZynqMP> 
```

### Update SPI-flash

Erase and write `len` bytes from memory at `addr` to flash at `offset`.

Usage:

```
sf update addr offset|partition len
```

e.g. Erase and write 0x10 bytes from memory at 0x4000 to flash at 0x3F00000.

```
ZynqMP> sf update 0x4000 0x3F00000 0x10
device 0 offset 0x3f00000, size 0x10
16 bytes written, 0 bytes skipped in 0.41s, speed 372 B/s
ZynqMP> 
```

# Direct access SPI device on source code

Sometime, developer need access SPI device on source code, and more detial see
`spi.c`.
