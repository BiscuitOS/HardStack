.word [中文教程](https://biscuitos.github.io/blog/GNUASM-.word/)
--------------------------------------------------

This directive expects zero or more expressions, of any section, 
separated by commas. The size of the number emitted, and its byte 
order, depend on what target computer the assembly is for.

```
  Warning: Special Treatment to support Compilers
```

Machines with a 32-bit address space, but that do less than 32-bit 
addressing, require the following special treatment. If the machine
of interest to you does 32-bit addressing.
In order to assemble compiler output into something that works, as
occasionlly does strange things to ‘.word’ directives. Directives
of the form ‘.word sym1-sym2’ are often emitted by compilers as part
of jump tables. Therefore, when as assembles a directive of the form
‘.word sym1-sym2’, and the difference between sym1 and sym2 does not
fit in 16 bits, as creates a secondary jump table, immediately before
the next label. This secondary jump table is preceded by a short-jump
to the first byte after the secondary table. This short-jump prevents
the flow of control from accidentally falling into the new table. Inside
the table is a long-jump to sym2. The original ‘.word’ contains sym1 
minus the address of the long-jump to sym2.

If there were several occurrences of ‘.word sym1-sym2’ before the
secondary jump table, all of them are adjusted. If there was a 
‘.word sym3-sym4’, that also did not fit in sixteen bits, a long-jump
to sym4 is included in the secondary jump table, and the .word directives
are adjusted to contain sym3 minus the address of the long-jump to
sym4; and so on, for as many entries in the original jump table as 
necessary.


