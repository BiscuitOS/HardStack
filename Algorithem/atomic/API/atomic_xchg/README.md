atomic_xchg [中文教程](https://biscuitos.github.io/blog/ATOMIC_atomic_xchg/)
----------------------------------

```
atomic_xchg (ARMv7 Cotex-A9MP)

static inline unsigned long __xchg(unsigned long x, volatile void *ptr,
                                             int size)
{
     unsigned long ret;
     unsigned int tmp;

     asm volatile("@ __xchg4\n"
     "1:     ldrex   %0, [%3]\n"
     "       strex   %1, %2, [%3]\n"
     "       teq     %1, #0\n"
     "       bne     1b"
             : "=&r" (ret), "=&r" (tmp)
             : "r" (x), "r" (ptr)
             : "memory", "cc");

     return ret;
}
```

Exchange atomic_t value.

Context:

* Driver Files: atomic.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_XX) += atomic.o
```

Then, compile driver and dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
