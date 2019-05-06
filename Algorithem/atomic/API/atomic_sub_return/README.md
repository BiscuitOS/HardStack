atomic_sub_return [中文教程](https://biscuitos.github.io/blog/ATOMIC_atomic_sub_return/)
----------------------------------

```
atomic_sub (ARMv7 Cotex-A9MP)

static inline int atomic_sub_return(int i, atomic_t *v)
{
        unsigned long tmp;
        int result;

        prefetchw(&v->counter);
        __asm__ volatile ("\n\t"
        "@ atomic_sub\n\t"
"1:      ldrex   %0, [%3]\n\t"        @ result, tmp115
"        sub     %0, %0, %4\n\t"      @ result,
"        strex   %1, %0, [%3]\n\t"    @ tmp, result, tmp115
"        teq     %1, #0\n\t"          @ tmp
"        bne     1b"
         : "=&r" (result), "=&r" (tmp), "+Qo" (v->counter)
         : "r" (&v->counter), "Ir" (i)
         : "cc");

        return result;
}
```

Sub and return new atomic_t value.

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
