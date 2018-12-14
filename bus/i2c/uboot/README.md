I2C on Uboot
-----------------------------------------

# Usage on Uboot

Read help usermanual:

```
ZynqMP> help i2c
i2c - I2C sub-system

Usage:
i2c bus [muxtype:muxaddr:muxchannel] - show I2C bus info
crc32 chip address[.0, .1, .2] count - compute CRC32 checksum
i2c dev [dev] - show or set current I2C bus
i2c loop chip address[.0, .1, .2] [# of objects] - looping read of device
i2c md chip address[.0, .1, .2] [# of objects] - read from I2C device
i2c mm chip address[.0, .1, .2] - write to I2C device (auto-incrementing)
i2c mw chip address[.0, .1, .2] value [count] - write to I2C device (fill)
i2c nm chip address[.0, .1, .2] - write to I2C device (constant address)
i2c probe [address] - test for and show device(s) on the I2C bus
i2c read chip address[.0, .1, .2] length memaddress - read to memory
i2c write memaddress chip address[.0, .1, .2] length [-s] - write memory
          to I2C; the -s option selects bulk write in a single transaction
i2c reset - re-init the I2C Controller
i2c speed [speed] - show or set I2C bus speed
```

### Shor or set current I2C bus

Command:

```
i2c dev [dev]
```

Show current I2C bus.

```
ZynqMP> i2c dev
Current bus is 0
ZynqMP> 
```

Set current I2C bus.

```
ZynqMP> i2c dev 0
Current bus is 0
ZynqMP>
```

### Test for and show device(s) on the I2C bus

Command: 

```
i2c probe [address]
```

Probe all device on current I2C bus

```
ZynqMP> i2c probe
Valid chip addresses: 50 51
ZynqMP>
```

Probe specify I2C slave device on current I2C bus.

```
ZynqMP> i2c probe 0x50
Valid chip addresses: 50
ZynqMP> 
```

### Read from I2C device 

Command:

```
i2c md chip address[.0, .1, .2] [# of objects]
```

Read objects's bytes specify offset address from Slav device in byte. e.g. 
Read 1 byte content from i2c slave device which i2c address is 0x50 and offset
is 0x2.

```
ZynqMP> i2c md 0x50 0x2 1                                                       
0001: 30    0                                                                   
ZynqMP> 
```

### Write to I2C device (fill)

command:

```
i2c mw chip address[.0, .1, .2] value [count]
```

Write count's bytes to specify address. e.g.
Write "0x12" into into slave device which i2c address is 0x50 and offset is 
0x1.

```
ZynqMP> i2c md 0x50 0x1                                                         
0001: 00    .                                                                   
ZynqMP> i2c mw 0x50 0x1 0x12                                                    
ZynqMP> i2c md 0x50 0x1                                                         
0001: 12    .
```
