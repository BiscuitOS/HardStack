radix_tree_node_rcu_free [中文教程](https://biscuitos.github.io/blog/RADIX-TREE_SourceAPI/#radix_tree_node_rcu_free)
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

Free a node from Radix-tree.

