BiscuitOS-pci-device-QEMU-emulate Usermanual
--------------------------------

BiscuitOS support establish  from source code, you
can follow step to create execute file on BiscuitOS:

1. Download Source Code

```
make download
```

2. Prepare Dependents

```
make prepare
```

3. Uncompress

```
make tar
```

4. Configure

```
make configure
```

5. Compile

```
make
```

6. Install

```
make install
```

7. Pack image

```
make pack
```

8. Running

```
cd /home/buddy/xspace/OpenSource/BiscuitOS/BiscuitOS/output/linux-5.0-x86_64
./RunBiscuitOS.sh start
```

## Silence output information

```
export BS_SILENCE=true
```

## Link

[BiscuitOS-pci-device-QEMU-emulate](https://gitee.com/BiscuitOS_team/HardStack/raw/Gitee/Device-Driver/PCIe/BiscuitOS-pci-device-QEMU-emulate)


# Reserved by BiscuitOS :)
