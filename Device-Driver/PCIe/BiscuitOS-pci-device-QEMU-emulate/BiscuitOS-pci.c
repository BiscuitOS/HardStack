/*
 * QEMU emulate PCI device for BiscuitOS
 *
 * BuddyZhang1 <buddy.zhang@aliyun.com>
 * 
 * BiscuitOS <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
 *
 * Copyright (c) 2012-2015 Jiri Slaby
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qapi/visitor.h"

/* PCI VENDOR:DEVICE ID */
#define BISCUITOS_PCI_VENDOR_ID	0x1016
#define BISCUITOS_PCI_DEVICE_ID	0x1413
/* BiscuitOS PCI QOM */
#define TYPE_PCI_BISCUITOS_DEVICE "BiscuitOS-pci"
#define BISCUITOS(obj)  OBJECT_CHECK(BiscuitOS_PCI_State, obj, TYPE_PCI_BISCUITOS_DEVICE)

#define FACT_IRQ        0x00000001

typedef struct {
    PCIDevice pdev;
    MemoryRegion io;
    MemoryRegion mmio;

    QemuThread thread;
    QemuMutex thr_mutex;
    QemuCond thr_cond;
    bool stopping;

    uint8_t addr;
    uint32_t fact;
#define BISCUITOS_STATUS_COMPUTING    0x01
#define BISCUITOS_STATUS_IRQFACT      0x80
    uint32_t status;

    uint32_t irq_status;
} BiscuitOS_PCI_State;

static bool BiscuitOS_msi_enabled(BiscuitOS_PCI_State *bps)
{
    return msi_enabled(&bps->pdev);
}

static void BiscuitOS_raise_irq(BiscuitOS_PCI_State *bps, uint32_t val)
{
    bps->irq_status |= val;
    if (bps->irq_status) {
        if (BiscuitOS_msi_enabled(bps)) {
            msi_notify(&bps->pdev, 0);
        } else {
            pci_set_irq(&bps->pdev, 1);
        }
    }
}

static uint64_t BiscuitOS_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    BiscuitOS_PCI_State *bps = opaque;
    uint64_t val = ~0ULL;

    if (size != 4 && size != 1) {
        return val;
    }

    switch (addr) {
    case 0x04:
        val = bps->addr;
        break;
    case 0x08:
        val = 'B';
        break;
    }

    return val;
}

static void BiscuitOS_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    BiscuitOS_PCI_State *bps = opaque;

    if (addr < 0x80 && size != 4 && size != 1) {
        return;
    }

    switch (addr) {
    case 0x04:
        bps->addr = (uint8_t)val;
        break;
    }
}

/* MMIO MR OPS */
static const MemoryRegionOps BiscuitOS_mmio_ops = {
    .read = BiscuitOS_mmio_read,
    .write = BiscuitOS_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 1,
    },
};

/* IO MR OPS */
static const MemoryRegionOps BiscuitOS_io_ops = {
    .read = BiscuitOS_mmio_read,
    .write = BiscuitOS_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 1,
    },
};

/*
 * We purposely use a thread, so that users are forced to wait for the status
 * register.
 */
static void *BiscuitOS_pci_thread(void *opaque)
{
    BiscuitOS_PCI_State *bps = opaque;

    while (1) {
        uint32_t val, ret = 1;

        qemu_mutex_lock(&bps->thr_mutex);
        while ((atomic_read(&bps->status) & BISCUITOS_STATUS_COMPUTING) == 0 &&
                        !bps->stopping) {
            qemu_cond_wait(&bps->thr_cond, &bps->thr_mutex);
        }

        if (bps->stopping) {
            qemu_mutex_unlock(&bps->thr_mutex);
            break;
        }

        val = bps->fact;
        qemu_mutex_unlock(&bps->thr_mutex);

        while (val > 0) {
            ret *= val--;
        }

        /*
         * We should sleep for a random period here, so that students are
         * forced to check the status properly.
         */

        qemu_mutex_lock(&bps->thr_mutex);
        bps->fact = ret;
        qemu_mutex_unlock(&bps->thr_mutex);
        atomic_and(&bps->status, ~BISCUITOS_STATUS_COMPUTING);

        if (atomic_read(&bps->status) & BISCUITOS_STATUS_IRQFACT) {
            qemu_mutex_lock_iothread();
            BiscuitOS_raise_irq(bps, FACT_IRQ);
            qemu_mutex_unlock_iothread();
        }
    }

    return NULL;
}

static void BiscuitOS_pci_realize(PCIDevice *pdev, Error **errp)
{
    BiscuitOS_PCI_State *bps = BISCUITOS(pdev);
    uint8_t *pci_conf = pdev->config;

    pci_config_set_interrupt_pin(pci_conf, 1);

    if (msi_init(pdev, 0, 1, true, false, errp)) {
        return;
    }

    qemu_mutex_init(&bps->thr_mutex);
    qemu_cond_init(&bps->thr_cond);
    qemu_thread_create(&bps->thread, "BiscuitOS-pci", BiscuitOS_pci_thread,
                       bps, QEMU_THREAD_JOINABLE);

    /* PCI IO BAR */
    memory_region_init_io(&bps->io, OBJECT(bps), &BiscuitOS_io_ops, bps,
                    "BiscuitOS-io", 128);
    /* PCI MMIO BAR */
    memory_region_init_io(&bps->mmio, OBJECT(bps), &BiscuitOS_mmio_ops, bps,
                    "BiscuitOS-mmio", 1 * MiB);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_IO, &bps->io);
    pci_register_bar(pdev, 1, PCI_BASE_ADDRESS_SPACE_MEMORY, &bps->mmio);
}

static void BiscuitOS_pci_uninit(PCIDevice *pdev)
{
    BiscuitOS_PCI_State *bps = BISCUITOS(pdev);

    qemu_mutex_lock(&bps->thr_mutex);
    bps->stopping = true;
    qemu_mutex_unlock(&bps->thr_mutex);
    qemu_cond_signal(&bps->thr_cond);
    qemu_thread_join(&bps->thread);

    qemu_cond_destroy(&bps->thr_cond);
    qemu_mutex_destroy(&bps->thr_mutex);

    msi_uninit(pdev);
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
