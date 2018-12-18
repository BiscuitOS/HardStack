Misc driver
----------------------------------

### Compile driver

```
make clean
make
```

### Running driver

```
sudo insmod misc.ko
cat /dev/misc_demo
```

### Remove driver

```
sudo rmmod misc
```
