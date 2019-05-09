XA_STATE [中文教程](https://biscuitos.github.io/blog/XARRAY_XA_STATE/)
----------------------------------

Declare an XArray operation state.

The xa_state is opaque to its users.  It contains various different pieces
of state involved in the current operation on the XArray.  It should be
declared on the stack and passed between the various internal routines.
The various elements in it should not be accessed directly, but only
through the provided accessor functions.  The below documentation is for
the benefit of those working on the code, not for users of the XArray.

@xa_node usually points to the xa_node containing the slot we're operating
on (and @xa_offset is the offset in the slots array).  If there is a
single entry in the array at index 0, there are no allocated xa_nodes to
point to, and so we store %NULL in @xa_node.  @xa_node is set to
the value %XAS_RESTART if the xa_state is not walked to the correct
position in the tree of nodes for this operation.  If an error occurs
during an operation, it is set to an %XAS_ERROR value.  If we run off the
end of the allocated nodes, it is set to %XAS_BOUNDS.

```
struct xa_state {
        struct xarray *xa;
        unsigned long xa_index;
        unsigned char xa_shift;
        unsigned char xa_sibs;
        unsigned char xa_offset;
        unsigned char xa_pad;           /* Helps gcc generate better code */
        struct xa_node *xa_node;
        struct xa_node *xa_alloc;
        xa_update_node_t xa_update;
};
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
