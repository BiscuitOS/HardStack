.endr [中文教程](https://biscuitos.github.io/blog/GNUASM-.endr/)
--------------------------------------------------

### .endr

Repeat the sequence of lines between the .endr directive and the next 
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
