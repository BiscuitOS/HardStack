MDIO (SMI/MIIM)
--------------------------------

## Term

* MDIO: Management Data Input/Output.

* SMI: Serial Management Interface. 

* MIIM: Media Independent Interface Management.

* MAC: Media Access Control.

* PHY: Ethernet physical layer.

* MII: Media Independent Interface.

* RMII: Reduced media-independent interface.

* GMII: Gigabit media-independent interface.

* RGMII: Reduced Gigabit media-independent interface.

* XGMII: 10-Gigabit media-independent interface.

* SGMII: Serial Gigabit media-independent interface.

* XAUI: 10 Gigabit Attachment Unit Interface.

* RXAUI: Reduced Pin eXtended Attachment Unit Interface.

# MII

```
+---------+                  +---------+                  +---------+
|         |                  |         |       MII        |         |
|   CPU   |<---------------->|   MAC   |<---------------->|   PHY   |
|         |                  |         |                  |         |
+---------+                  +---------+                  +---------+
```

The media-independent interface (MII) was originally defined as a standard 
interface to connect a Fast Ethernet media access control (MAC) block to a PHY
chip. The MII is standardized by IEEE 802.3u and connects different types of
PHY ot MACs. Thus any MAC may be used with any PHY, independent of the network
signal transmission media.

The Management Data Input/Output (MDIO) serial bus is subset of the MII that is
used to transfer management information between MAC and PHY. At power up, using
Auto-Negotiation, the PHY usually adapts to whatever it is connected to unless
settings are altered via the MDIO interface.

The original MII transfers network data using 4-bit nibbles in each direction 
(4 transmit data bit, 4 receive data bits). The data is locked at 25 MHz to 
achieve 100Mbit/s throughput. The original MII design has been extened to
support reduced signals and increased speeds. Current variants include RMII,
GMII, RGMII, XGMII, SGMII.

MII has two signal interfaces:

* A Data interface to the Ethernet MAC, for sending and receiving Ethernet
  from data.

* A PHY management interface, MDIO, used to read and write the control and 
  status registers of the PHY in order to configure each PHY before operation,
  and to monitor link status during operation.

```
      MAC                                              PHY
+--------------+                                 +--------------+
|              |                                 |              |
|              |       TX_ER/TX_EN/TXD[3:0]      |              |
|             -|-------------------------------->|    RX+/-     |
|              |                                 |              |
|   MII Data   |   RX_ER/RX_DV/RX_CLK/RXD[3:0]   |              |
|              |<--------------------------------|-   TX+/-     |
|              |          CRS/COL/TX_CLK         |              |
|              |<--------------------------------|-             |
+--------------+                                 +--------------+
|              |                                 |              |
|              |               MDC               |  Registers   |
|             -|-------------------------------->|              |
|     MIIM     |                                 |              |
|              |<------------------------------->|              |
|              |               MDIO              |              |
|              |                                 |              |
+--------------+                                 +--------------+
```
# MDIO

Management Data Input/Output (MDIO), also known as Serial Management Interface
(SMI) or Media Independent Interface Management (MIIM), is a serial bus defined
for the Ethernet family of IEEE 802.3 standards for the media independent 
interface, or MII. The MII connects Media Access Control (MAC) device with
Ethernet physical layer (PHY) circuits. The MAC device controlling the MDIO is
called the Static Management Entify (SME).

The MDIO interface is implemented by two signals:

* MDC clock: driven by the MAC device to the PHY.

* MDIO data: bindirectional, the PHY drives it to provide register data at the
  end of a read operation.

The bus only supports a single MAC as the master, and can have up to 32 PHY
slave.

### Bus Timing (Clause 22)

Before a register access, PHY devices generally require a preamble of 32 ones
to sent by the MAC on the MDIO line. The access consists of 16 control bits,
followed by 16 data bits. The control bits consist of 2 start bits, 2 access
byte bits (read or write), the PHY address (5 bits), the register address (
5bits), and 2 "turnaroud" bits. During a write command, the MAC provides
address and data. For a read command, the PHY takes over the MDIO line during
the turnaround bit times, supplies the MAC with the register data requested,
then release the MDIO line.

MDIO Read

![MDIO_timing](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000071.png)

MDIO Write

![MDIO_timing](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/DEV000072.png)

### Bus Timing (Clause 45)

Clause 45 provides extension of Clause 22 MDC/MDIO management interface to
access more device registers while retaining its logical compatibility of the
frame format. Clause 22 uses frame format with "Start of Frame" code of `01` 
while Clause 45 uses frame format with "Start of Frame" code of `00`.

# Usage of MDIO

MDIO is utilized on Uboot, kernel, userspace, and Ariduino. Details see:

* Usage on uboot: uboot/README.md

* Usage on kernel: kernel/README.md

* Usage on userspace: user/README.md

* Usage on Arduino: arduino/README.md

## Reference

[Media-independent interface](https://en.wikipedia.org/wiki/Media-independent_interface)

[Management Data Input/Output](https://en.wikipedia.org/wiki/Management_Data_Input/Output)

[MDIO Clause 22 and 45](www.ieee802.org/3/efm/public/nov02/oam/pannell_oam_1_1102.pdf)
