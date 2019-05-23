RB-Tree Erase Cause Color Flip
-------------------------------------------

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
0x1 0x2 0x3 0x4 0x5 0x7 0x8 0x9 0x129 
Re- Iterate over RBTree
0x2 0x3 0x4 0x5 0x7 0x8 0x9 0x129
```
