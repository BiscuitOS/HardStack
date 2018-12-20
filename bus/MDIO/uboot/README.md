MDIO (SMI/MIIM) on Uboot
----------------------------------

On Uboot, MAC(CPU) is able to configure a spcical PHY via SMI or obtain newest
status from PHY via SMI. The uboot offer `mii` tools or MII API to configure
PHY. The details information as follow:

# mii

The `mii` is a default tools on mainstream distro Uboot, developer use it under
command mode on Uboot. The usage of `mii` as follow:

```
mii - MII utility commands                                                                                
Usage:
mii device                            - list available devices
mii device <devname>                  - set current device
mii info   <addr>                     - display MII PHY info
mii read   <addr> <reg>               - read  MII PHY <addr> register <reg>
mii write  <addr> <reg> <data>        - write MII PHY <addr> register <reg>
mii modify <addr> <reg> <data> <mask> - modify MII PHY <addr> register <reg>
                                        updating bits identified in <mask>
mii dump   <addr> <reg>               - pretty-print <addr> <reg> (0-5 only)
```

### list available devices

`mii device` will list all available devices.

Usage:

```
mii device
```

e.g. On Soc, PHY is 1512

```
ZynqMP> mii device
MII devices: 'eth0'
Current device: 'eth0'
ZynqMP> 
```

### Display PHY information

`mii info` is able to display PHY information

Usage: 

```
mii info <addr>
```

The `addr` is PHY ID. e.g.:

```
ZynqMP> mii info 0
PHY 0x00: OUI = 0x5043, Model = 0x1D, Rev = 0x01, 1000baseT, FDX
ZynqMP>
```

### Read

`mii read` is used to read register from PHY. 

Usage:

```
mii read <phy_id> <reg_id>
```

e.g. Read ID regsiter from PHY 0. The PHY is mv88e1512, and the ID register
is 0x2 and 0x3. The PHY Identifier 1 register is 0x2 and the value alway 
`0x141`. The PHY Identifier 2 register is 0x3 and the value is `0x0DDx`.

```
ZynqMP> mii read 0 0x3
0DD1
ZynqMP> mii read 0 0x2
0141
ZynqMP> 
```

### Write

`mii write` is used to write data into PHY register.

Usage:

```
mii write <phy_id> <reg_id> <data>
```

e.g. Write 1 into Register 22 on PHY0.

```
ZynqMP> mii read 0 0x16                                                         
0000                                                                            
ZynqMP> mii write 0 0x16 0x1                                                    
ZynqMP> mii read 0 0x16                                                         
0001                                                                            
ZynqMP> 
```

# Access PHY on Source code

It's necessary to access PHY on source code, Here offer a demo code which 
access PHY via MDIO. see `mdio.c`
