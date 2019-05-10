BBXX [中文教程](https://biscuitos.github.io/blog/RCU_BBXX/)
----------------------------------

RCU.

Context:

* Driver Files: rcu.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += rcu.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
