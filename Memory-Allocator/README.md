Memory types and attributes and the memory order model
----------------------------------------------------------------

ARMv6 defined a set of memory attributes with the characteristics required 
to support the memory and devices in the system memory map. In ARMv7 this
set of attributes is extended by the addition of the Outer Shareable 
attribute for Normal memory and, in an implementation that does not include
the Large Physical Address Extension, for Device memory.

##### Note

Whether an ARMv7 implementation distinguishes between Inner Shareable and
Outer Shareable memory is IMPLEMENTATION DEFINED.


The ordering of accesses for regions of memory, referred to as the memory
order model, is defined by the memory attributes. This model is described
in the following sections:

* [Memory types](#Memory types)

* [Summary of ARMv7 memory attribute](#ARMv7 memory attribute)

* [Atomicity in the ARM architecture](#Atomicity)

* [Concurrent modification and execution of instruction](#Concurrent ME)

* [Normal memory](#Normal memory)

* [Device and Strongly-ordered memory](#DeviceStrongly)

* [Memory access restrictions](#Access Restrictions)

### <span id="Memory types">Memory types</span>

For each memory region, the most significant memory attribute specifies the
memory type. There are three mutually exclusive memory  types:

> - Normal
>
> - Device
>
> - Strongly-ordered

Normal and Device memory regions have additional attributes.

Usually, memory used for programs and for data storages is suitable for
access using the Normal memory attribute. Examples of memory technologies
for which the Normal memory attribute is appropriate are:

> - programmed Flash ROM
> 
> - ROM
>
> - SRAM
>
> - DRAM and DDR memory.

```
Note:

During programming, Flash memory can be ordered more strictly than Normal memory.
```

System peripherals (I/O) generally conform to different access rules. Examples
of I/O accesses are:

```
•   FIFOs where consecutive accesses

    ----    add queued value on write accesses.
    ----    remove queued values on read accesses.

•   interrupt controller registers where an access can be used as an
    interrupt acknowledge, changing the state of the controller itself.

•   memory controller configuration register that are used for setting up
    the timing and correctness of areas of Normal memory.

•   memory-mapped peripherals, where accessing a memory location can cause
    side-effects in the system.
```

In ARMv7, the Strongly-ordered or Device memory attribute provides suitable
access control for such peripherals. To ensure correct system behavior, the
access rules for Device and Strongly-ordered memory are more restrictive
than those for Normal memory, so that:

```
•   Nether read nor write accesses can be performed speculatively.

    ---Note---
    However, translation table walks can be made speculatively to memory
    marked as Device or Strongle-ordered.
    ----------

•   Read and write accesses cannot be repeated, for example, on return from
    an exception.

•   The number, order and sizes of the accesses are maintained.
```

### <span id="ARMv7 memory attribute">Summary of ARMv7 memory attribute</span>




### <span id="Normal memory">Normal memory</span>

Accesses to normal memory region are idempotent, meaning that they exhibit
the following properies:

```
•    read accesses can be repeated with no side-effects

•    repeated read accesses return the last value written to the resource
     being read

•    read accesses can fetch additional memory location with no side-effects

•    write accesses can be repeated with no side-effects in the following
     cases:
     
     ----     if the contents of the location accessed are unchanged between
              the repeated writes
     ----     as the result of an exception, as described in this section

•    unaligned access can be support

•    accesses can be merged before accessing the target memory system.
```

Normal memory can be read/write or read-only, and a Normal memory region is
defined as being either Shareable or Non-shareable. For Shareable Normal
memory, whether a VMSA implementation distinguishes between Inner Shareable
and Outer Shareable is IMPLEMENTATION DEFINED. A PMSA implementation makes
no distinction between Inner Shareable and Outer Shareable regions.

```
VMSA Normal Memory:

0                                                          4G
+----------------+-----------------------------------------+
|                |                                         |
|                |                                         |
|                |                                         |
+----------------+-----------------------------------------+
        |                            |
        |                            |
        |                            |
        V                            |
+----------------+                   |
|                |                   |
|                |                   |
|                |                   |
+----------------+                   |
Non-shareable memory                 |
                                     |
                                     |
                                     V  Shareable memory
                 +-------------------+---------------------+
                 |                   |                     |
                 |                   |                     |
                 |                   |                     |
                 +-------------------+---------------------+
                          |                     |
                          |                     |
                          |                     |
                          V                     |
                 +-------------------+          |
                 |                   |          |
                 |                   |          |
                 |                   |          |
                 +-------------------+          |
                 Inner Shareable memory         |
                                                |
                                                V
                                     +----------------------+
                                     |                      |
                                     |                      |
                                     |                      |
                                     +----------------------+
                                     Outer Shareable memory



PMSA Normal Memory:

0                                                          4G
+----------------+-----------------------------------------+
|                |                                         |
|                |                                         |
|                |                                         |
+----------------+-----------------------------------------+
        |                            |
        |                            |
        |                            |
        V                            |
+----------------+                   |
|                |                   |
|                |                   |
|                |                   |
+----------------+                   |
Non-shareable memory                 |
                                     |
                                     |
                                     V 
                 +-----------------------------------------+
                 |                                         |
                 |                                         |
                 |                                         |
                 +-----------------------------------------+
                 Shareable memory
```                                  

The Normal memory type attribute applies to most memory used in a system.
Accesses to Normal memory have a weakly consistent memory models, for
example Memory Consistency Models for Shared Memory-Multiprocessors. In
general, for Normal memory, barrier operations are required where the order
of memory accesses observed by other observers must be controlled. This
requirement applies regardless of the cacheability and shareability 
attributes of the Normal memory region.

The ordering requirements of accesses described in Ordering requirements 
for memory accesses apply to all explicit accesses.

An instruction that generates a sequence of accesses as described in 
Atomicity in the ARM architecture might be abandonded as a result of an
exception being taken during the sequence of accesses. On return from the
exception the instruction is restarted, and therefore one or more of the
memory location might be accessed multiple times. This can result in 
repeated write accesses to a location that has been changed between the
write accesses.

The architecture permits speculative access to memory locations marked as
Normal if the access permissions and domain permit an access to the
locations. 

A Normal memory region has shareability attributes that define the data
coherency properties of the region. These attributes do not affect the 
coherency requirements of:

```
•    Instruction fetches, see Instruction coherency issues.

•    Translation table walk for VMSA implementation of:

     ----   ARMv7-A without the Multiprocessing extensions

     ----   versions of the architecture before ARMv7.

     For more information see ELB maintenance operations and the memory
     order model.
```

##### Non-shareable Normal memory

For a Normal memory region, the Non-Shareable attribute identifies Normal
memory that is likely to be accessed only by a single processor. A region
of Normal memory with the Non-shareable attribute does not have any 
requirement to make data accesses by different observers coherent, unless
the memory is Non-cacheable. If other observers share the memory system,
software must use cache maintenance operations if the presence of cache
might lead to coherency issue when communicating between the obervers. This
cache maintenance requirement is in addition to the barrier operations
that are required to ensure memory ordering.

For Non-shareable Normal memory, it is IMPLEMENTATION DEFINED whether the 
Load-Exclusive and Store-Exclusive synchronization primitives take account
of the possibility of accesses by more than one observer.

##### Shareable, Inner Shareable, and Outer Shareable Normal memory

For Normal memory, the Shareable and Outer Shareable memory attribute 
describe Normal memory that expected to be accessed by multiple processor
or other system masters:

```
•    In a VMSA implementation, Normal memory that has the Shareable 
     attribute but not the Outer Shareable attribute assigned is 
     described as having the Inner Shareable attribute.

•    In a PMSA implementation, no distinction is made between Inner
     Shareable and Outer Shareable Normal memory.
```

A region of Normal memory with the Shareable attribute is one for which
data accesses to memory by different observers within the same shareability
domain are coherent.

The Outer Shareable attribute is introduced in ARMv7, and can be applied
only to a Normal memory region in a VMSA implementation that has the 
Shareable attribute assigned. It creates three levels of shareability for
a Normal memory region:

* Non-shareable

  A Normal memory region that doesn't have the Shareable attribute assigned.

* Inner Shareable

  A Normal memory region that has the Shareable attribute assigned, but not
  the Outer Shareable attribute.

* Outer Shareable

  A Normal memory region that has both the Shareable and the Outer Shareable
  attributes assigned.

These attribute can define sets of observers for which the shareability 
attributes make the data or unified caches transparent for data accesses.
The sets of observers that are affected by the shareability attributes are
described as shareability domains. The details of the use of these attributes
are system-specific. Example table shows how they might be used:

```
In a VMSA implementation, a particular subsystem with two clusters of 
processors has the requirement that:

•    in each cluster, the data or unified caches of the processors in the
     cluster are transparent for all data accesses with the Inner Shareable
     attribute

•    however, between the two clusters, the caches:

     ----     are not transparent for data accesses that have only the Inner
              Shareable attribute.
     ----     are transparent for data accesses that have the Outer Shareable
              attribute.

In this system, each cluster is in a different shareability domain for the
Inner Shareable attribute, but all components of the subsystem are in the 
same shareability domain for the Outer Shareable attribute.

A system might implement two such subsystems. If the data or unified caches
of one subsystem are not transparent to the accesses from the other
subsystem, this system has two Outer Shareable shareability domains.
```






























