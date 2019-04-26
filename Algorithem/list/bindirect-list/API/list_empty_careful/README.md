list_empty_careful [中文教程](https://biscuitos.github.io/blog/LIST_list_empty_careful/)
----------------------------------

```
bidirect-list

+-----------+<--o    +-----------+<--o    +-----------+<--o    +-----------+
|           |   |    |           |   |    |           |   |    |           |
|      prev |   o----| prev      |   o----| prev      |   o----| prev      |
| list_head |        | list_head |        | list_head |        | list_head |
|      next |---o    |      next |---o    |      next |---o    |      next |
|           |   |    |           |   |    |           |   |    |           |
+-----------+   o--->+-----------+   o--->+-----------+   o--->+-----------+
```

Tests whether a list is empty and not being modified, and checks that no
other CPU might be in the process of modifying either member (next or prev).

Context:

* Driver Files: list.c

## Usage

Copy Driver Files into `/drivers/xxx/`, and modify Makefile on current 
directory, as follow:

```
obj-$(CONFIG_XX) += list.o
```

Then, compile driver and dts. Details :

```
make
```

## Running

Packing image and runing on the target board.
