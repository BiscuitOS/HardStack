// SPDX-License-Identifier: GPL-2.0
/*
 * PCI BAR Mapping IO-Port or MMIO
 *
 * BuddyZhang1 <buddy.zhang@aliyun.com>
 * BiscuitOS <http://biscuitos.cn/blog/BiscuitOS_Catalogue/>
 *
 * Copyright (c) 2012-2015 Jiri Slaby
 */
#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qapi/visitor.h"

/* PCI VENDOR:DEVICE ID */
#define BISCUITOS_PCI_VENDOR_ID		0x1016
#define BISCUITOS_PCI_DEVICE_ID		0x1413
/* PCI BAR Layout */
#define BAR_IO				0x00
#define BAR_MMIO			0x01
#define BAR_SIZE			0x1000
#define SLOT_NUM_REG			0x00
#define SLOT_SEL_REG			0x04
#define MIN_FREQ_REG			0x08
#define MAX_FREQ_REG			0x0C

/* BiscuitOS PCI QOM */
#define TYPE_PCI_BISCUITOS_DEVICE 	"BiscuitOS-PCI-BAR"
#define BISCUITOS(obj)  		\
	OBJECT_CHECK(BiscuitOS_PCI_State, obj, TYPE_PCI_BISCUITOS_DEVICE)


typedef struct {
	PCIDevice pdev;
	MemoryRegion io;	/* IO-BAR */
	MemoryRegion mmio;	/* MMIO-BAR */
} BiscuitOS_PCI_State;

/* IO-BAR and MMIO-BAR Shared Memory */
static int BAR_BUFFER[4] = { 1, 1, 0x40, 0x60 };

static uint64_t BiscuitOS_bar_read(void *opaque, hwaddr addr, unsigned size)
{
	uint64_t val = ~0ULL;

	switch (addr) {
	case SLOT_NUM_REG:
		val = BAR_BUFFER[0];
		break;
	case SLOT_SEL_REG:
		val = BAR_BUFFER[1];
		break;
	case MIN_FREQ_REG:
		val = BAR_BUFFER[2];
		break;
	case MAX_FREQ_REG:
		val = BAR_BUFFER[3];
		break;
	default:
		break;
	}

	return val;
}

static void BiscuitOS_bar_write(void *opaque, hwaddr addr, 
					uint64_t val, unsigned size)
{
	switch (addr) {
	case SLOT_NUM_REG:
		BAR_BUFFER[0] = val;
		break;
	case SLOT_SEL_REG:
		BAR_BUFFER[1] = val;
		break;
	case MIN_FREQ_REG:
		BAR_BUFFER[2] = val;
		break;
	case MAX_FREQ_REG:
		BAR_BUFFER[3] = val;
		break;
	default:
		break;
	}
}

/* MMIO MR OPS */
static const MemoryRegionOps BiscuitOS_mmio_ops = {
	.read	= BiscuitOS_bar_read,
	.write	= BiscuitOS_bar_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.impl = {
		.min_access_size = 4, /* READ/WRITE 32bit */
		.max_access_size = 4, /* READ/WRITE 32bit */
	},
};

/* IO MR OPS */
static const MemoryRegionOps BiscuitOS_io_ops = {
	.read	= BiscuitOS_bar_read,
	.write	= BiscuitOS_bar_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.impl = {
		.min_access_size = 1, /* IN/OUT 8bit */
		.max_access_size = 4, /* IN/OUT 32bit */
	},
};

static void BiscuitOS_pci_realize(PCIDevice *pdev, Error **errp)
{
	BiscuitOS_PCI_State *bps = BISCUITOS(pdev);

	/* PCI IO BAR */
	memory_region_init_io(&bps->io, OBJECT(bps), &BiscuitOS_io_ops, bps,
				"BiscuitOS-BAR-IO", BAR_SIZE);
	/* PCI MMIO BAR */
	memory_region_init_io(&bps->mmio, OBJECT(bps), &BiscuitOS_mmio_ops, bps,
				"BiscuitOS-BAR-MMIO", BAR_SIZE);
	pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_IO, &bps->io);
	pci_register_bar(pdev, 1, PCI_BASE_ADDRESS_SPACE_MEMORY, &bps->mmio);
}

static void BiscuitOS_pci_uninit(PCIDevice *pdev) { }

static void BiscuitOS_class_init(ObjectClass *class, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(class);
	PCIDeviceClass *k = PCI_DEVICE_CLASS(class);

	k->realize   = BiscuitOS_pci_realize;
	k->exit      = BiscuitOS_pci_uninit;
	k->vendor_id = BISCUITOS_PCI_VENDOR_ID;
	k->device_id = BISCUITOS_PCI_DEVICE_ID;
	k->revision  = 0x10;
	k->class_id  = PCI_CLASS_OTHERS;
	set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static void BiscuitOS_pci_register_types(void)
{
	static InterfaceInfo interfaces[] = {
		{ INTERFACE_CONVENTIONAL_PCI_DEVICE },
		{ },
	};
	static const TypeInfo BiscuitOS_pci_info = {
		.name          = TYPE_PCI_BISCUITOS_DEVICE,
		.parent        = TYPE_PCI_DEVICE,
		.instance_size = sizeof(BiscuitOS_PCI_State),
		.class_init    = BiscuitOS_class_init,
		.interfaces = interfaces,
	};

	type_register_static(&BiscuitOS_pci_info);
}
type_init(BiscuitOS_pci_register_types)
