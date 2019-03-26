.type [中文教程](https://biscuitos.github.io/blog/GNUASM-.type/)
--------------------------------------------------

### .type

This directive, permitted only within .def/.endef pairs, records the integer 
int as the type attribute of a symbol table entry.

‘.type’ is associated only with COFF format output; when as is configured 
for b.out output, it accepts this directive but ignores it.
