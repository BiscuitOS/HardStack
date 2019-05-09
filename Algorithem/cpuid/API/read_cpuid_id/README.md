read_cpuid_id [中文教程](https://biscuitos.github.io/blog/CPUID_read_cpuid_id/)
----------------------------------

Read CPUID register.

Context:

* Driver Files: cpuid.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_CPUID_XX) += cpuid.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
