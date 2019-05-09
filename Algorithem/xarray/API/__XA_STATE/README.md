__XA_STATE [中文教程](https://biscuitos.github.io/blog/XARRAY___XA_STATE/)
----------------------------------

Declare an XArray operation state.

```
#define __XA_STATE(array, index, shift, sibs)  {        \
        .xa = array,                                    \
        .xa_index = index,                              \
        .xa_shift = shift,                              \
        .xa_sibs = sibs,                                \
        .xa_offset = 0,                                 \
        .xa_pad = 0,                                    \
        .xa_node = XAS_RESTART,                         \
        .xa_alloc = NULL,                               \
        .xa_update = NULL                               \
}
```

Context:

* Driver Files: xarray.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_SPINLOCK_XX) += xarray.o
```

Then, compile driver or dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
