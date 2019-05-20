2-3 Tree
---------------------------------

![](https://img.shields.io/aur/license/yaourt.svg)

## Description

A 2-3 tree implemented in C.

## Dependencies

* This program readily compiles with the latest version of gcc. I do not 
  use any non-standard features or extensions, so any version of gcc should
  suffice.

## Usage (for now)

* tree23.h/tree23.c

  Core file for 2-3 Tree.

* tree23_run.c

  Describe how to use 2-3 tree in your program.

* tree23_perform.c

  The 2-3 Tree performance test.

First, call `make`, then:

```
make
```

##### Normal use

```
$ ./tree23
Itervate over 2-3 Tree.
ldata: 1
rdata: 23
ldata: 56
ldata: 56
ldata: 67
ldata: 87
rdata: 90
ldata: 344
ldata: 423
rdata: 568
ldata: 876
ldata: 987
ldata: 3245
rdata: 8976
rdata: 12334
ldata: 76542
Re- Itervate over 2-3 Tree
ldata: 1
rdata: 23
ldata: 56
ldata: 56
ldata: 67
ldata: 87
ldata: 90
ldata: 344
rdata: 568
ldata: 987
ldata: 3245
ldata: 8976
rdata: 12334
ldata: 76542

```

##### Performance test

```
./tree23_perform [num_to_insert & num_to_delete] [filename]
```

You must specify both the number of items to insert and delete, otherwise the
program will run the standard test of 100,000 insertions followed by 50,000
deletions. At the end, the values left within the tree are printed, so you
can verify the tree's correctness.

If you want to only see whether the tree's insertion works or not, specify
0 as the number of elements you wish to delete.

Lastly, `[filename]` specifies an output file you can print the values that
will be deleted to, if you wish. All values are floating point, and
randomly generated. It is completely optional however and `./tree23_perform` 
with the first two arguments will run with what you've given it.

## History

In the year 2013, after completing my Data Structures course, I figured that
I ought to implement some of the more complex items we went over in class but
only touched on at a high level. I found the 2-3 tree to be very cool and 
thought it incredible that one could search for any element contained within 
in O(logn) iterations.

I set out to implement this tree, but was baffled by several cases my 
specific design would have to consider. Thus, I was unable to complete
the project before my first semester at UTSA, and now, two years later, 
I have given it a go again.

## References

[GitHub: PySean/tree23](https://github.com/PySean/tree23)

[Wikipedia Entry](https://en.wikipedia.org/wiki/2%E2%80%933_tree)

[USC PDF on 2-3 tree deletion](http://www-bcf.usc.edu/~dkempe/CS104/11-19.pdf)
