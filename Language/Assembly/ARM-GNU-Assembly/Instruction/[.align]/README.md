.align [中文教程](https://biscuitos.github.io/blog/GNUASM-.align/)
--------------------------------------------------

### .align

Syntax: .align alignment[, [fill][, max]]

Pad the location counter (in the current subsection) to a particular storage boundary. alignment (which must be absolute) is the alignment required, as described below.

fill (also absolute) gives the fill value to be stored in the padding bytes. It (and the comma) may be omitted. If it is omitted, the padding bytes are normally zero. However, on some systems, if the section is marked as containing code and the fill value is omitted, the space is filled with no-op instructions (I didn't checked whether this is the case in TIGCC).

max is also absolute, and is also optional. If it is present, it is the maximum number of bytes that should be skipped by this alignment directive. If doing the alignment would require skipping more bytes than the specified maximum, then the alignment is not done at all. You can omit the fill value (the second argument) entirely by simply using two commas after the required alignment; this can be useful if you want the alignment to be filled with no-op instructions when appropriate.

The way the required alignment is specified varies from system to system. For the a29k, hppa, m68k, m88k, w65, sparc, Xtensa, and Renesas / SuperH SH, and i386 using ELF format, the first expression is the alignment request in bytes. For example .align 8 advances the location counter until it is a multiple of 8. If the location counter is already a multiple of 8, no change is needed.

For other systems, including the i386 using a.out format, and the arm and strongarm, it is the number of low-order zero bits the location counter must have after advancement. For example .align 3 advances the location counter until it a multiple of 8. If the location counter is already a multiple of 8, no change is needed.

This inconsistency is due to the different behaviors of the various native assemblers for these systems which as must emulate. as also provides .balign and .p2align directives, which have a consistent behavior across all architectures (but are specific to as).
