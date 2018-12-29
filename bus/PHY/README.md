PHY
------------------------------

# Term

* PCS: Physical Coding Sublayer

* PMD: Physical Medium Dependent

* LLC: Logical Link Control Sublayer

* MAC: Media Access Control Sublayer

* RS: Reconciliation Sublayer

* PMA: Physical Medium Attachment Sublayer

* MII: Media Independent Interface

* Copper: Copper cable

* Fiber: Fiber cable

* SERDES: Serializer/Deserializer

* RJ45: A standard for a physical interface between customer wiring and 
        telephone company wiring.

* SFP: Small form-factor pluggable

# PHY

PHY is an abbreviation for the physical layer of the OSI model and refers to
the circuitry required to implement physical layer functions.

A PHY connects a link layer device (often called MAC as an acronym for medium
access control) to a physical medium such as an optical fiber or copper. A
PHY device typically includes both Physical Coding Sublayer (PCS) and Physical
Medium Dependent (PMD) layer function.

```
+---------+        +---------+              +---------+
|         |        |         |     MII      |         |
|   CPU   |<------>|   MAC   |<------------>|   PHY   |<------> Fiber/Copper
|         |        |         |              |         |
+---------+        +---------+              +---------+
```

# PHY Example uses

* Ethernet

  A PHY chip (PHYceiver) is commonly found on Ethernet devices. Its purpose is
  to provide analog signal physcial access to the link. It is usually used in 
  conjunction with a Media Independent Interface (MII) chip or interfaced to a
  microcontroller that takes care of the higher layer functions.

* Universal Serial Bus (USB)

  A PHY chip is integrated into most USB controllers in hosts or embedded
  system and provides the bridge between the digital and modulated parts of 
  the interface.

* Wireless LAN or Wi-Fi

  The PHY portion consists of the RF, mixed-signal and analog portions, that
  are often transceivers, and the digital baseband portion that use digital
  signal processor (DSP) and communication algorithm processing, including 
  channel codes. It is common that these PHY portions are integrated with the
  media access control (MAC) layer in System-on-a-chip (SOC) implementations.
  Other similar wireless application are 3G/4G/LTE, WiMAX, UWB, etc.

# PCS and PMD

The Physical Coding Sublayer (PCS) is a networking protocol sublayer in the
Fast Ethernet, Gigabit Ethernet, and 10 Gigabit Ethernet standards. It resides
at the top of the physical layer (PHY), and provides an interface between the
Physical Medium Attachment (PMA) sublayer and the Media Independent Interface (
MII). It is responsible for data encoding and decoding, scrambling and 
descrambling, alignment marker insertion and removal, block and symbol 
redistribution, and lane block synchronization and deskew.

The Physical Medium Dependent sublayer or PMDs further help to define the
physical layer of computer network protocols. They define the details of 
transmission and reception of individual bits on a physical medium. These
responsibilities encompass bit timing, signal encoding, interacting with the
physical medium, and the properties of the cable, optical fiber, or wire 
itself. Common example are specifications for Fast Ethernet, Gigabit Ethernet
and 10 Gigabit Ethernet defined by the Institute of Electrical and Electronics
Engineers (IEEE).

The Ethernet PCS sublayer is at the top of the Ethernet physical layer (PHY).
The Ethernet PMD sublayer is part of the Ethernet physical layer (PHY). The hierarchy is as follow:

### Data Link Layer (Layer 2)

* LLC (Logical Link Control Sublayer)

* MAC (Media Access Control Sublayer)

* RS (Reconciliation Sublayer) - This sublayer processes PHY Local/Remote Fault
  messages and handles DDR conversion.

### PHY layer (Layer 1)

* PCS (Physcial Coding Sublayer) - this sublayer determines when a functional
  link has been established, provides rate difference compensation, and 
  performs coding.

* PMA (Physical Medium Attachment Sublayer) - This sublayer performs PMA
  framing, octet synchronization/detection.

* PMD (Physical Medium Dependent Sublayer) - This sublayer consists of a 
  transceiver for the physical medium.


# Reference

[PHY on Wikipedia](https://en.wikipedia.org/wiki/PHY_(chip))

[PCS: Physcial Coding Sublayer](https://en.wikipedia.org/wiki/Physical_Coding_Sublayer)

[PMD: Physical Medium Dependent Sublayer](https://en.wikipedia.org/wiki/Physical_Medium_Dependent)


