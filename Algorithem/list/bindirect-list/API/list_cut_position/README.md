list_cut_position [中文教程](https://biscuitos.github.io/blog/LIST_list_cut_position/)
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

Cut a list into two.

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
