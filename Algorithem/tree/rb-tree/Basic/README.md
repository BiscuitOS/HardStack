RB-Tree Usermanual
-------------------------------------------

```
RB-Tree

                                                       [] Black node
                                                       () Red node
                   [4]
                    |
         o----------o----------o
         |                     |
        (2)                   (7)
         |                     |
  o------o------o      o-------o-------o
  |             |      |               |
 [1]           [3]    [5]             [9]
                                       |
                               o-------o-------o
                               |               |
                              (8)            (129)

```

#### File list

* rbtree.c

  The core library of rbtree.

* rbtree.h

  The header file of rbtree.

* rb_run.c

  The userspace demo code which descibe how to use rbtree.

#### Usage

Run 'make' command to compile source code, detail as follow:

```
make clean
make
./rbtree
```

Output:

```
Iterate over RBTree.
0x1 0x2 0x3 0x5 0x7 0x8 0x9 0x129 
Iterate over by postorder.
0x1 0x3 0x2 0x7 0x129 0x9 0x8 0x5
