spin_trylock [中文教程](https://biscuitos.github.io/blog/SPINLOCK_spin_trylock/)
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

Try to obtain spinlock.

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
