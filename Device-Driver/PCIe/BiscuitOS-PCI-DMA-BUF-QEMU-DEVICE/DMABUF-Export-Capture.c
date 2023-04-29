// SPDX-License-Identifier: GPL-2.0
/*
 * DMA-BUF Export Video Capture
 *
 * BuddyZhang1 <buddy.zhang@aliyun.com>
 * BiscuitOS <http://biscuitos.cn/blog/BiscuitOS_Catalogue/>
 *
 * Copyright (c) 2012-2015 Jiri Slaby
 */
#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/msix.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qapi/visitor.h"

/* BiscuitOS PCI QOM */
#define TYPE_PCI_BISCUITOS_DEVICE 	"BiscuitOS-DMA-BUF-EXPORT"
#define BISCUITOS(obj)  		\
	OBJECT_CHECK(BiscuitOS_PCI_State, obj, TYPE_PCI_BISCUITOS_DEVICE)

/* PCI VENDOR:DEVICE ID */
#define BISCUITOS_PCI_VENDOR_ID		0x1018
#define BISCUITOS_PCI_DEVICE_ID		0x1991
/* PCI BAR Layout */
#define BAR_IO				0x00
#define BAR_MSIX			0x01
#define MSIX_SIZE			0x1000
#define BAR_SIZE			0x20
#define DOORBALL_REG			0x10
/* PCI Status */
#define BSTATUS_COMPUTING		0x01
/* DMA Register Layout */
#define DMA_SRC_REG			0x00
#define DMA_DST_REG			0x04
#define DMA_DIR_REG			0x08
#define DMA_LEN_REG			0x0C
#define PCI_TO_DDR			0x00
#define DDR_TO_PCI			0x01
#define DMA_FW_LEN			0x2000

static const char *DMA_FW[3] = { "CA[0] Weclome to BiscuitOS Video Capture!\n", 
			    	 "CA[1] http://www.biscuitOS.cn\n",
                            	 "CA[2] DMABUF-GPUB STREAM\n" };
static int dma_count;

typedef struct {
	PCIDevice	pdev;
	MemoryRegion	io;	/* IO-BAR */
	MemoryRegion	mmio;	/* MSIX-BAR */

	/* Interrupt Thread */
	QemuThread	thread;
	QemuMutex	thr_mutex;
	QemuCond	thr_cond;

	/* Status */
	uint32_t	status;
	bool		stopping;

	/* DMA */
	char		*dma_buffer;
	struct {
		dma_addr_t src;
		dma_addr_t dst;
		dma_addr_t dir;
		dma_addr_t len;
	} dma;
	QEMUTimer	dma_timer;
} BiscuitOS_PCI_State;

static void BiscuitOS_raise_irq(BiscuitOS_PCI_State *bps)
{
	if (msix_enabled(&bps->pdev))
		msix_notify(&bps->pdev, 0);
	else
		pci_irq_assert(&bps->pdev);
}

static void BiscuitOS_bar_write(void *opaque, hwaddr addr, 
					uint64_t val, unsigned size)
{
	BiscuitOS_PCI_State *bps = opaque;

	switch (addr) {
	case DMA_SRC_REG:
		bps->dma.src = val;
		break;
	case DMA_DST_REG:
		bps->dma.dst = val;
		break;
	case DMA_DIR_REG:
		bps->dma.dir = val;
		break;
	case DMA_LEN_REG:
		bps->dma.len = val;
		break;
	case DOORBALL_REG: /* Kick off */
		if (atomic_read(&bps->status) & BSTATUS_COMPUTING)
			break;
		qemu_mutex_lock(&bps->thr_mutex);
		atomic_or(&bps->status, BSTATUS_COMPUTING);
		qemu_cond_signal(&bps->thr_cond);
		qemu_mutex_unlock(&bps->thr_mutex);
		break;
	default:
		break;
	}
}

/* IO MR OPS */
static const MemoryRegionOps BiscuitOS_io_ops = {
	.write	= BiscuitOS_bar_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
	.impl = {
		.min_access_size = 1, /* IN/OUT min 8bit */
		.max_access_size = 4, /* IN/OUT max 32bit */
	},
};

/* MMIO MR OPS */
static const MemoryRegionOps BiscuitOS_mmio_ops = {
	.endianness = DEVICE_NATIVE_ENDIAN,
	.impl = {
		.min_access_size = 8, /* MOV min 64bit */
		.max_access_size = 8, /* MOV max 64bit */
	},
};

/* Interrupt Thread */
static void *BiscuitOS_Interrupt_Thread(void *opaque)
{
	BiscuitOS_PCI_State *bps = opaque;

	while (1) {
		qemu_mutex_lock(&bps->thr_mutex);
		while ((atomic_read(&bps->status) &
				BSTATUS_COMPUTING) == 0 && !bps->stopping)
			qemu_cond_wait(&bps->thr_cond, &bps->thr_mutex);

		if (bps->stopping) {
			qemu_mutex_unlock(&bps->thr_mutex);
			break;
		}

		/* DMA */
		timer_mod(&bps->dma_timer,
			qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL));
		qemu_mutex_unlock(&bps->thr_mutex);
		atomic_and(&bps->status, ~BSTATUS_COMPUTING);
	}
	return NULL;
}

static void BiscuitOS_dma_timer(void *opaque)
{
	BiscuitOS_PCI_State *bps = opaque;

	if (bps->dma.dir == PCI_TO_DDR) { /* TLP Memory Read */
		sprintf(bps->dma_buffer, "%s", DMA_FW[dma_count++]);
		printf("[QEMU Video Capture] %s", bps->dma_buffer);
		pci_dma_write(&bps->pdev, bps->dma.dst,
				bps->dma_buffer + bps->dma.src, bps->dma.len);
	} else { /* TLP Memory Write */
		pci_dma_read(&bps->pdev, bps->dma.src,
				bps->dma_buffer + bps->dma.dst, bps->dma.len);
	}
	/* Raise MSIX Interrupt */
	BiscuitOS_raise_irq(bps);
	atomic_and(&bps->status, ~BSTATUS_COMPUTING);
}

static void BiscuitOS_pci_realize(PCIDevice *pdev, Error **errp)
{
	BiscuitOS_PCI_State *bps = BISCUITOS(pdev);

	/* PCI IO BAR */
	memory_region_init_io(&bps->io, OBJECT(bps), &BiscuitOS_io_ops, bps,
				"BiscuitOS-BAR-IO", BAR_SIZE);
	/* PCI MMIO BAR */
	memory_region_init_io(&bps->mmio, OBJECT(bps), &BiscuitOS_mmio_ops, bps,
				"BiscuitOS-MSIX-BAR", MSIX_SIZE);
	pci_register_bar(pdev, BAR_IO, PCI_BASE_ADDRESS_SPACE_IO, &bps->io);
	pci_register_bar(pdev, BAR_MSIX,
				PCI_BASE_ADDRESS_SPACE_MEMORY, &bps->mmio);

	qemu_mutex_init(&bps->thr_mutex);
	qemu_cond_init(&bps->thr_cond);
	qemu_thread_create(&bps->thread, "BiscuitOS-PCI-DMA-MSIX",
			BiscuitOS_Interrupt_Thread, bps, QEMU_THREAD_JOINABLE);

	/* MSIX TABLE */
	if (msix_init_exclusive_bar(pdev, 8, BAR_MSIX, errp)) {
		qemu_log("MSXI Table init error.\n");
		return;
	}
	msix_vector_use(pdev, 0);

	/* DMA */
	bps->dma_buffer = malloc(DMA_FW_LEN);
	if (!bps->dma_buffer) {
		qemu_log("DMA BUFFER ALLOC Failed.");
		return;
	}
	timer_init_ms(&bps->dma_timer, QEMU_CLOCK_VIRTUAL,
					BiscuitOS_dma_timer, bps);
}

static void BiscuitOS_pci_uninit(PCIDevice *pdev)
{
	BiscuitOS_PCI_State *bps = BISCUITOS(pdev);

	qemu_mutex_lock(&bps->thr_mutex);
	bps->stopping = true;
	qemu_mutex_unlock(&bps->thr_mutex);
	qemu_cond_signal(&bps->thr_cond);
	qemu_thread_join(&bps->thread);

	free(bps->dma_buffer);
	qemu_cond_destroy(&bps->thr_cond);
	qemu_mutex_destroy(&bps->thr_mutex);
}

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
