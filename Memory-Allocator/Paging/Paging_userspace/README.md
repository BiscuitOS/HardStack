MEMBLOCK Memory Allocator on Userspace
--------------------------------------------

##### usage

```
make
./biscuitos
```

standard output:

```
$ ./biscuitos 
BiscuitOS - MEMBLOCK Memory Allocator
Physcial Memory Range: 0x60000000 - 0x61000000
Virtual Memory Ranges: 0x7f1899659010 - 0x7f189a659010
BiscuitOS-90
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60ffffe0 - 0x61000000
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
BP Physical Address: 0x60ffffe0
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0 - 0
BiscuitOS-phys:
Phys 0x60ffffe0
Virt 0x7f189a658ff0
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60ffffe0 - 0x61000000
Alloc from top to bottom
BP[0] Physical Address: 0x60ffffe0
BP[1] Physical Address: 0x60ffffc0
BP[2] Physical Address: 0x60ffffa0
BP[3] Physical Address: 0x60ffff80
BP[4] Physical Address: 0x60ffff60
Alloc from bottom to top
BP[5] Physical Address: 0x60000000
BP[6] Physical Address: 0x60000020
BP[7] Physical Address: 0x60000040
BP[8] Physical Address: 0x60000060
BP[9] Physical Address: 0x60000080
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60000000 - 0x600000a0
Reserve Memory Region: 0x60ffff60 - 0x61000000
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60000000 - 0x60001000
Reserve Memory Region: 0x60002000 - 0x60003000
Reserve Memory Region: 0x60004000 - 0x60005000
Reserve Memory Region: 0x60006000 - 0x60007000
Reserve Memory Region: 0x60008000 - 0x60009000
Reserve Memory Region: 0x6000a000 - 0x6000b000
Reserve Memory Region: 0x6000c000 - 0x6000d000
Reserve Memory Region: 0x6000e000 - 0x6000f000
Reserve Memory Region: 0x60010000 - 0x60011000
Reserve Memory Region: 0x60012000 - 0x60013000
Reserve Memory Region: 0x60014000 - 0x60015000
Reserve Memory Region: 0x60016000 - 0x60017000
Reserve Memory Region: 0x60018000 - 0x60019000
Reserve Memory Region: 0x6001a000 - 0x6001b000
Reserve Memory Region: 0x6001c000 - 0x6001d000
Reserve Memory Region: 0x6001e000 - 0x6001f000
Reserve Memory Region: 0x60020000 - 0x60021000
Reserve Memory Region: 0x60022000 - 0x60023000
Reserve Memory Region: 0x60024000 - 0x60025000
Reserve Memory Region: 0x60026000 - 0x60027000
Reserve Memory Region: 0x60028000 - 0x60029000
Reserve Memory Region: 0x6002a000 - 0x6002b000
Reserve Memory Region: 0x6002c000 - 0x6002d000
Reserve Memory Region: 0x6002e000 - 0x6002f000
Reserve Memory Region: 0x60030000 - 0x60031000
Reserve Memory Region: 0x60032000 - 0x60033000
Reserve Memory Region: 0x60034000 - 0x60035000
Reserve Memory Region: 0x60036000 - 0x60037000
Reserve Memory Region: 0x60038000 - 0x60039000
Reserve Memory Region: 0x6003a000 - 0x6003b000
Reserve Memory Region: 0x6003c000 - 0x6003d000
Reserve Memory Region: 0x6003e000 - 0x6003f000
Trigger memblock_double_array()
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60000000 - 0x60001000
Reserve Memory Region: 0x60002000 - 0x60003000
Reserve Memory Region: 0x60004000 - 0x60005000
Reserve Memory Region: 0x60006000 - 0x60007000
Reserve Memory Region: 0x60008000 - 0x60009000
Reserve Memory Region: 0x6000a000 - 0x6000b000
Reserve Memory Region: 0x6000c000 - 0x6000d000
Reserve Memory Region: 0x6000e000 - 0x6000f000
Reserve Memory Region: 0x60010000 - 0x60011000
Reserve Memory Region: 0x60012000 - 0x60013000
Reserve Memory Region: 0x60014000 - 0x60015000
Reserve Memory Region: 0x60016000 - 0x60017000
Reserve Memory Region: 0x60018000 - 0x60019000
Reserve Memory Region: 0x6001a000 - 0x6001b000
Reserve Memory Region: 0x6001c000 - 0x6001d000
Reserve Memory Region: 0x6001e000 - 0x6001f000
Reserve Memory Region: 0x60020000 - 0x60021000
Reserve Memory Region: 0x60022000 - 0x60023000
Reserve Memory Region: 0x60024000 - 0x60025000
Reserve Memory Region: 0x60026000 - 0x60027000
Reserve Memory Region: 0x60028000 - 0x60029000
Reserve Memory Region: 0x6002a000 - 0x6002b000
Reserve Memory Region: 0x6002c000 - 0x6002d000
Reserve Memory Region: 0x6002e000 - 0x6002f000
Reserve Memory Region: 0x60030000 - 0x60031000
Reserve Memory Region: 0x60032000 - 0x60033000
Reserve Memory Region: 0x60034000 - 0x60035000
Reserve Memory Region: 0x60036000 - 0x60037000
Reserve Memory Region: 0x60038000 - 0x60039000
Reserve Memory Region: 0x6003a000 - 0x6003b000
Reserve Memory Region: 0x6003c000 - 0x6003d000
Reserve Memory Region: 0x6003e000 - 0x6003f000
Reserve Memory Region: 0x60040000 - 0x60041000
Reserve Memory Region: 0x60042000 - 0x60043000
Reserve Memory Region: 0x60044000 - 0x60045000
Reserve Memory Region: 0x60046000 - 0x60047000
Reserve Memory Region: 0x60048000 - 0x60049000
Reserve Memory Region: 0x6004a000 - 0x6004b000
Reserve Memory Region: 0x6004c000 - 0x6004d000
Reserve Memory Region: 0x6004e000 - 0x6004f000
Reserve Memory Region: 0x60050000 - 0x60051000
Reserve Memory Region: 0x60052000 - 0x60053000
Reserve Memory Region: 0x60fff000 - 0x60fff2ff
Free All.....
Valid Memory Regions:  0x60000000 - 0x61000000
Reserve Memory Region: 0x60fff000 - 0x60fff2ff
```
