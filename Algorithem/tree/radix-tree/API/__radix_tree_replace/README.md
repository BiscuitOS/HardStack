\_\_radix_tree_replace [中文教程](https://biscuitos.github.io/blog/RADIX-TREE_SourceAPI/#__radix_tree_replace)
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

Replace a node on Radix-tree.

