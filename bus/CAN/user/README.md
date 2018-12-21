CAN on userspace
---------------------------------

On userspace, developer have two way to access CAN device, as follow:

* Access CAN device on Application

* Access CAN device via tools.

## Access CAN device via tools

Linux offer `canutils` and `ip` tools to access CAN device. It's easy to access
CAN device through those tools. The usage of tools as follow:

#### Start CAN device

```
ifconfig can0 up
```

#### Configure CAN device

```
ip link set can0 up type can bitrate 1000000

or

canconfig can0 restart-ms 1000 bitrate 10000000 ctrmode triple-sampling on
canconfig can0 start
```

#### Send CAN frame

```
cansend can0 7ff@898787
```

#### Monitor CAN frame

```
candump can0
```

#### Stop CAN device

```
canconfig can0 stop

or 

ifconfig can0 down
```

# Access CAN device on Application

Here's has offer two source code `can_tx.c` and `can_rx.c`, developer can 
reference this code on your application to sent or receive CAN frame.

#### Usage

* Send CAN frame: can-tx.c

* Receive CAN frame: can-rx.c
