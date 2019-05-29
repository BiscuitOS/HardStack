radix_tree_next_chunk [中文教程](https://biscuitos.github.io/blog/RADIX-TREE_radix_tree_next_chunk/)
----------------------------------

```
Radix-tree                                             RADIX_TREE_MAP: 6
                                 (root)
                                   |
                         o---------o---------o
                         |                   |
                       (0x0)               (0x2)
                         |                   |
                 o-------o------o            o---------o
                 |              |                      |
               (0x0)          (0x2)                  (0x2)
                 |              |                      |
        o--------o------o       |             o--------o--------o
        |               |       |             |        |        |
      (0x0)           (0x1)   (0x0)         (0x0)    (0x1)    (0x3)
        A               B       C             D        E        F

A: 0x00000000
B: 0x00000001
C: 0x00000080
D: 0x00080080
E: 0x00080081
F: 0x00080083
```

This radix tree iterator works in terms of "chunks" of slots.  A chunk is a
subinterval of slots contained within one radix tree leaf node.  It is
described by a pointer to its first slot and a struct radix_tree_iter
which holds the chunk's position in the tree and its size.  For tagged
iteration radix_tree_iter also holds the slots' bit-mask for one chosen
radix tree tag.


Context:

* Driver Files: radix.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += radix.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
