spin_unlock [中文教程](https://biscuitos.github.io/blog/SPINLOCK_spin_unlock/)
----------------------------------

```
Memory access:

     +----------+
     |          |
     | Register |                                         +--------+
     |          |                                         |        |
     +----------+                                         |        |
           A                                              |        |
           |                                              |        |
+-----+    |      +----------+        +----------+        |        |
|     |<---o      |          |        |          |        |        |
| CPU |<--------->| L1 Cache |<------>| L2 Cache |<------>| Memory |
|     |<---o      |          |        |          |        |        |
+-----+    |      +----------+        +----------+        |        |
           |                                              |        |
           o--------------------------------------------->|        |
                        volatile/atomic                   |        |
                                                          |        |
                                                          +--------+

```

Release a spinlock.

Context:

* Driver Files: spinlock.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += spinlock.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
