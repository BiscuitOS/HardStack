CMA Usage
-----------------------------------------

In order to use CMA memory, we need offer a CMA driver and CMA application,
`drv/cma.c` is CMA driver, and `app/cma.c` is CMA application.

## Kernel 

On kernel, we need add cma information on dts, as follow:

```
/ {
	chosen {
		bootargs = "... cma=125M";
	}
};
```

or if kernel doesn't dts, we can setup macro `CONFIG_CMA_SIZE_MBYTES` as
follow:

```
CONFIG_CMA_SIZE_MBYTES=125
```

## Userland

On userland, and compile and running cma application where in `app/cma.c`.
