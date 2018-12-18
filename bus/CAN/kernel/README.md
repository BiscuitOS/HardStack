CAN on Kernel
-----------------------------------------

* can.c

  A simple can driver frame, only register a CAN device.

* can_adv.c

  A perfect can driver frame which contains transmit and receive CAN frame.

### Usage

Open Kernel macro on configure stage, as follow:

```
CONFIG_CAN=y
CONFIG_CAN_RAW=y
CONFIG_CAN_BCM=y
CONFIG_CAN_GW=y
CONFIG_CAN_DEV=y
CONFIG_CAN_CALC_BITTIMING=y
```

And U can configure kernel as follow:

```
make menuconfig ARCH=arm64
```

**Networking support -->**

![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000065.png)

**CAN bus subsystem support -->**

![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000066.png)

Select all and enter **CAN Device Drivers -->**

![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000067.png)

Select **CAN bit-timing calculation**

![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000068.png)

Then compile source code as follow:

```
make clean
make
```

Final, U can install module into running system as follow:

```
sudo insmod can.ko
```

On userspace, U can utilize **net-tools** to verify CAN interface. And as
follow:

```
root@BiscuitOS:~# ifconfig -a
can0      Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          UP RUNNING NOARP  MTU:16  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:10 
          RX bytes:0 (0.0 B)  TX bytes:1224 (0.0 B)
```
