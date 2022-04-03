/*
 * QEMU emulate PCI device for BiscuitOS
 *
 * BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_PCI_VENDOR_ID	0x1016
#define BISCUITOS_PCI_DEVICE_ID	0x1413
#define BISCUITOS_CFG_ID_ADDR	0x3000UL
#define BISCUITOS_CFG_ID_VAL	0x911016

#define TYPE_PCI_BISCUITOS_DEVICE "BiscuitOS-pci"
#define BISCUITOS(obj)  OBJECT_CHECK(BiscuitOS_PCI_State, obj, TYPE_PCI_BISCUITOS_DEVICE)

#define FACT_IRQ        0x00000001
#define DMA_IRQ         0x00000100

#define DMA_START       0x40000
#define DMA_SIZE        4096

typedef struct {
    PCIDevice pdev;
    MemoryRegion mmio;

    QemuThread thread;
    QemuMutex thr_mutex;
    QemuCond thr_cond;
    bool stopping;

    uint32_t addr4;
    uint32_t fact;
#define BISCUITOS_STATUS_COMPUTING    0x01
#define BISCUITOS_STATUS_IRQFACT      0x80
    uint32_t status;

    uint32_t irq_status;

#define BISCUITOS_DMA_RUN             0x1
#define BISCUITOS_DMA_DIR(cmd)        (((cmd) & 0x2) >> 1)
#define BISCUITOS_DMA_FROM_PCI        0
#define BISCUITOS_DMA_TO_PCI          1
#define BISCUITOS_DMA_IRQ             0x4
    struct dma_state {
        dma_addr_t src;
        dma_addr_t dst;
        dma_addr_t cnt;
        dma_addr_t cmd;
    } dma;
    QEMUTimer dma_timer;
    char dma_buf[DMA_SIZE];
    uint64_t dma_mask;
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

static void BiscuitOS_lower_irq(BiscuitOS_PCI_State *bps, uint32_t val)
{
    bps->irq_status &= ~val;

    if (!bps->irq_status && !BiscuitOS_msi_enabled(bps)) {
        pci_set_irq(&bps->pdev, 0);
    }
}

static bool within(uint32_t addr, uint32_t start, uint32_t end)
{
    return start <= addr && addr < end;
}

static void BiscuitOS_check_range(uint32_t addr, uint32_t size1, uint32_t start,
                uint32_t size2)
{
    uint32_t end1 = addr + size1;
    uint32_t end2 = start + size2;

    if (within(addr, start, end2) &&
            end1 > addr && within(end1, start, end2)) {
        return;
    }

    hw_error("BiscuitOS: DMA range 0x%.8x-0x%.8x out of bounds (0x%.8x-0x%.8x)!",
            addr, end1 - 1, start, end2 - 1);
}

static dma_addr_t BiscuitOS_clamp_addr(const BiscuitOS_PCI_State *bps, dma_addr_t addr)
{
    dma_addr_t res = addr & bps->dma_mask;

    if (addr != res) {
        printf("BiscuitOS: clamping DMA %#.16"PRIx64" to %#.16"PRIx64"!\n", addr, res);
    }

    return res;
}

static void BiscuitOS_dma_timer(void *opaque)
{
    BiscuitOS_PCI_State *bps = opaque;
    bool raise_irq = false;

    if (!(bps->dma.cmd & BISCUITOS_DMA_RUN)) {
        return;
    }

    if (BISCUITOS_DMA_DIR(bps->dma.cmd) == BISCUITOS_DMA_FROM_PCI) {
        uint32_t dst = bps->dma.dst;
        BiscuitOS_check_range(dst, bps->dma.cnt, DMA_START, DMA_SIZE);
        dst -= DMA_START;
        pci_dma_read(&bps->pdev, BiscuitOS_clamp_addr(bps, bps->dma.src),
                bps->dma_buf + dst, bps->dma.cnt);
    } else {
        uint32_t src = bps->dma.src;
        BiscuitOS_check_range(src, bps->dma.cnt, DMA_START, DMA_SIZE);
        src -= DMA_START;
        pci_dma_write(&bps->pdev, BiscuitOS_clamp_addr(bps, bps->dma.dst),
                bps->dma_buf + src, bps->dma.cnt);
    }

    bps->dma.cmd &= ~BISCUITOS_DMA_RUN;
    if (bps->dma.cmd & BISCUITOS_DMA_IRQ) {
        raise_irq = true;
    }

    if (raise_irq) {
        BiscuitOS_raise_irq(bps, DMA_IRQ);
    }
}

static void dma_rw(BiscuitOS_PCI_State *bps, bool write, dma_addr_t *val, dma_addr_t *dma,
                bool timer)
{
    if (write && (bps->dma.cmd & BISCUITOS_DMA_RUN)) {
        return;
    }

    if (write) {
        *dma = *val;
    } else {
        *val = *dma;
    }

    if (timer) {
        timer_mod(&bps->dma_timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + 100);
    }
}

static uint64_t BiscuitOS_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    BiscuitOS_PCI_State *bps = opaque;
    uint64_t val = ~0ULL;

    if (size != 4) {
        return val;
    }

    switch (addr) {
    case 0x04:
        val = bps->addr4;
        break;
    case 0x08:
        qemu_mutex_lock(&bps->thr_mutex);
        val = bps->fact;
        qemu_mutex_unlock(&bps->thr_mutex);
        break;
    case 0x20:
        val = atomic_read(&bps->status);
        break;
    case 0x24:
        val = bps->irq_status;
        break;
    case 0x80:
        dma_rw(bps, false, &val, &bps->dma.src, false);
        break;
    case 0x88:
        dma_rw(bps, false, &val, &bps->dma.dst, false);
        break;
    case 0x90:
        dma_rw(bps, false, &val, &bps->dma.cnt, false);
        break;
    case 0x98:
        dma_rw(bps, false, &val, &bps->dma.cmd, false);
        break;
    case BISCUITOS_CFG_ID_ADDR:
        val = BISCUITOS_CFG_ID_VAL;
        break;
    }

    return val;
}

static void BiscuitOS_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    BiscuitOS_PCI_State *bps = opaque;

    if (addr < 0x80 && size != 4) {
        return;
    }

    if (addr >= 0x80 && size != 4 && size != 8) {
        return;
    }

    switch (addr) {
    case 0x04:
        bps->addr4 = ~val;
        break;
    case 0x08:
        if (atomic_read(&bps->status) & BISCUITOS_STATUS_COMPUTING) {
            break;
        }
        /* BISCUITOS_STATUS_COMPUTING cannot go 0->1 concurrently, because it is only
         * set in this function and it is under the iothread mutex.
         */
        qemu_mutex_lock(&bps->thr_mutex);
        bps->fact = val;
        atomic_or(&bps->status, BISCUITOS_STATUS_COMPUTING);
        qemu_cond_signal(&bps->thr_cond);
        qemu_mutex_unlock(&bps->thr_mutex);
        break;
    case 0x20:
        if (val & BISCUITOS_STATUS_IRQFACT) {
            atomic_or(&bps->status, BISCUITOS_STATUS_IRQFACT);
        } else {
            atomic_and(&bps->status, ~BISCUITOS_STATUS_IRQFACT);
        }
        break;
    case 0x60:
        BiscuitOS_raise_irq(bps, val);
        break;
    case 0x64:
        BiscuitOS_lower_irq(bps, val);
        break;
    case 0x80:
        dma_rw(bps, true, &val, &bps->dma.src, false);
        break;
    case 0x88:
        dma_rw(bps, true, &val, &bps->dma.dst, false);
        break;
    case 0x90:
        dma_rw(bps, true, &val, &bps->dma.cnt, false);
        break;
    case 0x98:
        if (!(val & BISCUITOS_DMA_RUN)) {
            break;
        }
        dma_rw(bps, true, &val, &bps->dma.cmd, true);
        break;
    }
}

static const MemoryRegionOps BiscuitOS_mmio_ops = {
    .read = BiscuitOS_mmio_read,
    .write = BiscuitOS_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
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

    timer_init_ms(&bps->dma_timer, QEMU_CLOCK_VIRTUAL, BiscuitOS_dma_timer, bps);

    qemu_mutex_init(&bps->thr_mutex);
    qemu_cond_init(&bps->thr_cond);
    qemu_thread_create(&bps->thread, "BiscuitOS-pci", BiscuitOS_pci_thread,
                       bps, QEMU_THREAD_JOINABLE);

    memory_region_init_io(&bps->mmio, OBJECT(bps), &BiscuitOS_mmio_ops, bps,
                    "BiscuitOS-mmio", 1 * MiB);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &bps->mmio);
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

    timer_del(&bps->dma_timer);
    msi_uninit(pdev);
}

static void BiscuitOS_obj_uint64(Object *obj, Visitor *v, const char *name,
                           void *opaque, Error **errp)
{
    uint64_t *val = opaque;

    visit_type_uint64(v, name, val, errp);
}

static void BiscuitOS_instance_init(Object *obj)
{
    BiscuitOS_PCI_State *bps = BISCUITOS(obj);

    bps->dma_mask = (1UL << 28) - 1;
    object_property_add(obj, "dma_mask", "uint64", BiscuitOS_obj_uint64,
                    BiscuitOS_obj_uint64, NULL, &bps->dma_mask, NULL);
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
        .instance_init = BiscuitOS_instance_init,
        .class_init    = BiscuitOS_class_init,
        .interfaces = interfaces,
    };

    type_register_static(&BiscuitOS_pci_info);
}
type_init(BiscuitOS_pci_register_types)
