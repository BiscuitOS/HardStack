BiscuitOS DMA module
============================

BiscuitOS DMA module is simple PCIe device with internal memory, which contains 2MiB RAM memory. The Vendor ID is **0x1014**, and Device ID is **0x1314**. The DMA contains an MMIO BAR, which mapping:

| Regisrer Name  | Address | Length | R/W | Describe                                    |
| :------------- | :------ | :----- | :-- | :------------------------------------------ |
| REG_DMA_STATUS | 0x20    | 4      | R   | The Status of DMA module                    |
| REG_INT_STATUS | 0x24    | 4      | R   | The Status of Interrupt                     |
| REG_PCI_BASE   | 0x60    | 4      | R   | The base address of PCI Memory              |
| REG_PCI_SIZE   | 0x64    | 4      | R   | The size of PCI Memory                      |
| REG_TRANS_SRC  | 0x68    | 4      | R/W | The transmit source address                 |
| REG_TRANS_DST  | 0x6c    | 4      | R/W | The transmit destination address            |
| REG_TRANS_CNT  | 0x70    | 4      | R/W | The number for transmitting                 |
| REG_TRANS_CMD  | 0x74    | 4      | R/W | The command for transmitting                |
| REG_TRANS_RUN  | 0x78    | 4      | R/W | Start to transmit                           |
| REG_INT_RAISE  | 0x80    | 4      | W   | Raise interrupt                             |
| REG_INT_DOWN   | 0x84    | 4      | W   | Drop interrupt                              |
