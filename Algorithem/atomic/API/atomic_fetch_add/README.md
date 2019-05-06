atomic_fetch_add [中文教程](https://biscuitos.github.io/blog/ATOMIC_atomic_fetch_add/)
----------------------------------

```
atomic_add_* (ARMv7 Cotex-A9MP)

static inline int atomic_fetch_add(int i, atomic_t *v)
{
        unsigned long tmp;
        int result, val;

        prefetchw(&v->counter);
        __asm__ volatile ("\n\t"
        "@ atomic_add\n\t"
"1:      ldrex   %0, [%4]\n\t"        @ result, tmp115
"        add     %1, %0, %5\n\t"      @ result,
"        strex   %2, %1, [%4]\n\t"    @ tmp, result, tmp115
"        teq     %2, #0\n\t"          @ tmp
"        bne     1b"
         : "=&r" (result), "=&r" (val), "=&r" (tmp), "+Qo" (v->counter)
         : "r" (&v->counter), "Ir" (i)
         : "cc");

        return result;
}
```

Obtain original valud and add it.

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
