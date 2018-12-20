MDIO (SMI/MIIM) on userspace
---------------------------------------

On userspace, system offers `mii-diag`, `mii-tool`, and `ethtool` to access
MDIO bus. The usage of these tools as follow:

## mii-diag

```
# mii-diag --help
Usage: mii-diag [-aDfrRvVw] [-AF <speed+duplex>] [--watch] <interface>.

  This program configures and monitors the transceiver management registers
  for network interfaces.  It uses the Media Independent Interface (MII)
  standard with additional Linux-specific controls to communicate with the
  underlying device driver.  The MII registers control and report network
  link settings and errors.  Examples are link speed, duplex, capabilities
  advertised to the link partner, status LED indications and link error
  counters.

   The common usage is
      mii-diag eth0

   The default interface is "eth0".
 Frequently used options are
   -A  --advertise <speed|setting>
   -F  --fixed-speed <speed>
        Speed is one of: 100baseT4, 100baseTx, 100baseTx-FD, 100baseTx-HD,
                         10baseT, 10baseT-FD, 10baseT-HD
   -s  --status     Return exit status 2 if there is no link beat.

 Less frequently used options are
   -a  --all-interfaces  Show the status all interfaces
              (Not recommended with options that change settings.)
   -D  --debug
   -g  --read-parameters        Get driver-specific parameters.
   -G  --set-parameters PARMS   Set driver-specific parameters.
       Parameters are comma separated, missing elements retain existing value.
   -M  --msg-level LEVEL        Set the driver message bit map.
   -p  --phy ADDR               Set the PHY (MII address) to report.
   -r  --restart        Restart the link autonegotiation.
   -R  --reset          Reset the transceiver.
   -v  --verbose        Report each action taken.
   -V  --version        Emit version information.
   -w  --watch          Continuously monitor the transceiver and report changes.

   This command returns success (zero) if the interface information can be
   read.  If the --status option is passed, a zero return means that the
   interface has link beat.
```

## mii-tools

```
mii-tool --help
usage: mii-tool [-VvRrwl] [-A media,... | -F media] [interface ...]
       -V, --version               display version information
       -v, --verbose               more verbose output
       -R, --reset                 reset MII to poweron state
       -r, --restart               restart autonegotiation
       -w, --watch                 monitor for link status changes
       -l, --log                   with -w, write events to syslog
       -A, --advertise=media,...   advertise only specified media
       -F, --force=media           force specified media technology
media: 1000baseTx-HD, 1000baseTx-FD,
       100baseT4, 100baseTx-FD, 100baseTx-HD,
       10baseT-FD, 10baseT-HD,
       (to advertise both HD and FD) 1000baseTx, 100baseTx, 10baseT
```

## ethtool

```
ethtool --help
```

# Access MDIO on application

On userspace, the system doesn't export common MDIO interface, so developer
can't access MDIO on common interface. If developer want to debug MDIO on 
userspace, the `mdio.c` offer some routine to access MDIO on userspace.

## Usage:

#### Compile and Install:

```
make clean
make
insmod mdio.ko
``` 

#### Read 

Read special PHY register via SMI/MDIO/MIIM.

Command:

```
echo "r,<phy_id>,<reg_id>," > /sys/devices/platform/mdio_demo/mdio_demo_reg
dmesg | tail -n 5
```

e.g. Read ID register for PHY 0

```
# echo "r,0x0,0x2," > /sys/devices/platform/mdio_demo/mdio_demo_reg
# dmesg | tail -n 5

Read: Port - Dev[0x0000] Reg[0x0002] Value[0x0141]

```

#### Write

Write data into speical PHY register via SMI/MDIO/MIIM.

Command:

```
echo "w,<phy_id>,<reg_id>,<data>," > /sys/devices/platform/mdio_demo/mdio_demo_reg
dmesg | tail -n 5
```

e.g. Write 0x48 into 0x16 register for PHY 0

```
# echo "r,0x0,0x16," > /sys/devices/platform/mdio_demo/mdio_demo_reg
# echo "w,0x0,0x16,0x48," > /sys/devices/platform/mdio_demo/mdio_demo_reg
# echo "r,0x0,0x16," > /sys/devices/platform/mdio_demo/mdio_demo_reg
# dmesg | tail -n 10

Read: Port - Dev[0x0000] Reg[0x0016] Value[0x0000]

Write: Port - Dev[0x0000] Reg[0x0016] Value[0x0048]

Read: Port - Dev[0x0000] Reg[0x0016] Value[0x0048]
```
#### Dump

Dump all PHY registers.

Command:

```
cat /sys/devices/platform/mdio_demo/mdio_demo_reg
```
