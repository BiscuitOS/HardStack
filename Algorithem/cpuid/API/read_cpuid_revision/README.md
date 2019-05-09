read_cpuid_revision [中文教程](https://biscuitos.github.io/blog/CPUID_read_cpuid_revision/)
----------------------------------

Read revision information.

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
