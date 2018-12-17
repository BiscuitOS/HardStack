I²C on Userland
------------------------------------

* I²C toolchain

* I²C application code

# I²C toolchain

The i2c-tools offers usefull tool to detect/probe/read/write on I²C bus. Tools
are: **i2cdump**, **i2cdetect**, **i2cget**, **i2cset**. Details as follow:

#### Install i2c-tools

Ubuntu directly use follow command to install i2c-tools:

```
sudo apt install i2c-tools
```

#### Detect I²C slave device

i2cdetect is a userspace program to scan an I2C bus for devices. It outputs a 
table with the list of detected devices on the specified bus. i2cbus indicates
the number or name of the I2C bus to be scanned, and should correspond to one
of the busses listed by i2cdetect -l. The optional parameters first and last
restrict the scanning range (default: from 0x03 to 0x77).

To detect i2c on specify I²C bus as follow:

```
Usage: i2cdetect [-F I2CBUS] [-l] [-y] [-a] [-q|-r] I2CBUS [FIRST LAST]
```

e.g. Detecting on I²C Bus 0.

```
# i2cdetect -y 0                                                                
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f                             
00:                                                                             
10:                                                                             
20:                                                                             
30: -- -- -- -- -- -- -- --                                                     
40:                                                                             
50: 50 51 -- -- -- -- -- -- -- -- -- -- -- -- -- --                             
60:                                                                             
70:
```

#### Examine I²C registers 

**i2cdump** is a small helper program to examine registers visible through
the I2C bus.

Examine specify I²C slave register.

```
Usage: i2cdump [-f] [-r FIRST-LAST] [-y] BUS ADDR [MODE] 
``` 

e.g. Dump slave device all register which I²C address is 0x50 in I²C Bus 0.

```
# i2cdump -f -y 0 0x50
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 35 02 32 52 00 02 00 02 ff ff ff ff ff ff ff ff    5?2R.?.?........
10: aa aa aa aa aa aa ff ff ff ff ff ff ff ff ff ff    ??????..........
20: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
30: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
40: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
50: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
60: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
70: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
80: 93 00 73 14 13 05 00 20 00 00 00 00 ff ff ff ff    ?.s???. ........
90: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
a0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
b0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
c0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
d0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
e0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
f0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................
# 

```

#### I²C read operations

**i2cget** is a small helper program to read registers visible through the I2C 
bus (or SMBus).

Utilize i2cget to read data from I²C Bus.

```
Usage: i2cget [-f] [-y] BUS CHIP-ADDRESS [DATA-ADDRESS [MODE]]
```

e.g. Read data from slave device which i2c address is 0x50 on I²C Bus 0.
The offset of register on Slave device is 0x80.

```
# i2cget -f -y 0 0x50 0x80 
0x93
```

#### I²C write operations

**i2cset** is a small helper program to set registers visible through the I2C
bus.

Utilize i2cset to write data into I²C Bus.

```
Usage: i2cset [-f] [-y] [-m MASK] BUS CHIP-ADDR DATA-ADDR [VALUE] ... [MODE]
```

e.g. Write 0x68 into slave device which i2c address is 0x50 on I²C Bus 0, and
the offset of register on slave device is 0x40.

```
# i2cget -f -y 0 0x50 0x40
0xff
# i2cset -f -y 0 0x50 0x40 0x68
# i2cget -f -y 0 0x50 0x40
0x68
```
-----------------------------------------------------------

# I²C application code

The common interface for I²C bus on userspace is **/dev/i2cx**, e.g. 
"/dev/i2c0" or "/dev/i2c1", the application can read and write data on I²C
bus via **/dev/i2cx**. Details see i2c.c


