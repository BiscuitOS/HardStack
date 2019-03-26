.rept [中文教程](https://biscuitos.github.io/blog/GNUASM-.rept/)
--------------------------------------------------

### .rept

Repeat the sequence of lines between the .rept directive and the next 
.endr directive count times.

```
For example, assembling

  .rept 3
  .long 3
  .endr

is equivalent to assembling

  .long 3
  .long 3
  .long 3
```
