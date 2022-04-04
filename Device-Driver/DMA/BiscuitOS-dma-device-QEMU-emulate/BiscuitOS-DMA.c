/*
 * QEMU emulate DMA device for BiscuitOS
 *
 * BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define BISCUITOS_DMA_VENDOR_ID	0x1016
#define BISCUITOS_DMA_DEVICE_ID	0x1314

#define TYPE_PCI_BISCUITOS_DEVICE "BiscuitOS-DMA"
#define BISCUITOS(obj)  \
        OBJECT_CHECK(BiscuitOS_DMA_State, obj, TYPE_PCI_BISCUITOS_DEVICE)

#define FACT_IRQ                0x00000001
#define DMA_IRQ                 0x00000100

#define DMA_BASE                0x40000
#define DMA_SIZE                (2 * 1024 * 1024) // 2MiB DMA
#define BISCUITOS_DMA_RUN       0x1
#define BISCUITOS_DMA_DIR(cmd)  (((cmd) & 0x2) >> 1)
#define BISCUITOS_DMA_TO_PCI    0
#define BISCUITOS_DMA_FROM_PCI  1
#define BISCUITOS_DMA_IRQ       0x8
#define BISCUITOS_FW            "This is BiscuitOS DMA module, welcome :)"
#define BISCUITOS_FW_OFFSET     (DMA_SIZE / 2)

#define BISCUITOS_STATUS_COMPUTING    0x01
#define BISCUITOS_STATUS_IRQFACT      0x80

/* Status Register */
#define REG_DMA_STATUS          0x20
#define REG_INT_STATUS          0x24

/* DMA Register */
#define REG_PCI_BASE            0x60
#define REG_PCI_SIZE            0x64
#define REG_TRANS_SRC           0x68
#define REG_TRANS_DST           0x6c
#define REG_TRANS_CNT           0x70
#define REG_TRANS_CMD           0x74
#define REG_TRANS_RUN           0x78

/* Interrupt Register */
#define REG_INT_RAISE           0x80
#define REG_INT_DOWN            0x84


typedef struct {
    PCIDevice pdev;
    MemoryRegion mmio;

    QemuThread thread;
    QemuMutex thr_mutex;
    QemuCond thr_cond;
    bool stopping;

    uint32_t fact;
    uint32_t status;
    uint32_t irq_status;

    /* DMA */
    struct dma_state {
        dma_addr_t src;
        dma_addr_t dst;
        dma_addr_t cnt;
        dma_addr_t cmd;
    } dma;
    QEMUTimer dma_timer;
    char dma_buf[DMA_SIZE];
    uint64_t dma_mask;
} BiscuitOS_DMA_State;

static bool BiscuitOS_msi_enabled(BiscuitOS_DMA_State *bps)
{
    return msi_enabled(&bps->pdev);
}

static void BiscuitOS_raise_irq(BiscuitOS_DMA_State *bps, uint32_t val)
{
    bps->irq_status |= val;
    if (bps->irq_status) {
        if (BiscuitOS_msi_enabled(bps))
            msi_notify(&bps->pdev, 0);
        else
            pci_set_irq(&bps->pdev, 1);
    }
}

static void BiscuitOS_lower_irq(BiscuitOS_DMA_State *bps, uint32_t val)
{
    bps->irq_status &= ~val;

    if (!bps->irq_status && !BiscuitOS_msi_enabled(bps))
        pci_set_irq(&bps->pdev, 0);
}

static bool within(uint32_t addr, uint32_t start, uint32_t end)
{
    return start <= addr && addr < end;
}

static void BiscuitOS_check_range(uint32_t addr, uint32_t size1,
                                  uint32_t start, uint32_t size2)
{
    uint32_t end1 = addr + size1;
    uint32_t end2 = start + size2;

    if (within(addr, start, end2) &&
                  end1 > addr && within(end1, start, end2))
        return;

    hw_error("BiscuitOS: DMA range 0x%.8x-0x%.8x out of "
             "bounds (0x%.8x-0x%.8x)!",
             addr, end1 - 1, start, end2 - 1);
}

static dma_addr_t BiscuitOS_clamp_addr(const BiscuitOS_DMA_State *bps,
                                       dma_addr_t addr)
{
    dma_addr_t res = addr & bps->dma_mask;

    if (addr != res)
        printf("BiscuitOS: clamping DMA %#.16"PRIx64" to %#.16"PRIx64"!\n",
							addr, res);

    return res;
}

static void BiscuitOS_dma_timer(void *opaque)
{
    BiscuitOS_DMA_State *bps = opaque;

    if (!(bps->dma.cmd & BISCUITOS_DMA_RUN))
        return;

    /* Mov data from DMA to PCI */
    if (BISCUITOS_DMA_DIR(bps->dma.cmd) == BISCUITOS_DMA_FROM_PCI) {
        uint32_t src = bps->dma.src;

        BiscuitOS_check_range(src, bps->dma.cnt, DMA_BASE, DMA_SIZE);
        src -= DMA_BASE;
        pci_dma_write(&bps->pdev, BiscuitOS_clamp_addr(bps, bps->dma.dst),
                bps->dma_buf + src, bps->dma.cnt);
    } else { /* Mov data from PCI to DMA */
        uint32_t dst = bps->dma.dst;

        BiscuitOS_check_range(dst, bps->dma.cnt, DMA_BASE, DMA_SIZE);
        dst -= DMA_BASE;
        pci_dma_read(&bps->pdev, BiscuitOS_clamp_addr(bps, bps->dma.src),
                bps->dma_buf + dst, bps->dma.cnt);
    }

    bps->dma.cmd &= ~BISCUITOS_DMA_RUN;
    if (bps->dma.cmd & BISCUITOS_DMA_IRQ) /* Raise Interrupt */
        BiscuitOS_raise_irq(bps, DMA_IRQ);
}

static void dma_rw(BiscuitOS_DMA_State *bps, bool write,
                      dma_addr_t *val, dma_addr_t *dma, bool timer)
{
    if (write && (bps->dma.cmd & BISCUITOS_DMA_RUN))
        return;

    if (write)
        *dma = *val;
    else
        *val = *dma;

    if (timer)
        timer_mod(&bps->dma_timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + 100);
}

static uint64_t BiscuitOS_mmio_read(void *opaque, hwaddr addr, unsigned size)
{
    BiscuitOS_DMA_State *bps = opaque;
    uint64_t val = ~0ULL;

    if (size != 4)
        return val;

    switch (addr) {
    case REG_DMA_STATUS:
        val = atomic_read(&bps->status);
        break;
    case REG_INT_STATUS:
        val = bps->irq_status;
        break;
    case REG_PCI_BASE:
        val = DMA_BASE;
        break;
    case REG_PCI_SIZE:
        val = DMA_SIZE;
        break;
    case REG_TRANS_SRC:
        dma_rw(bps, false, &val, &bps->dma.src, false);
        break;
    case REG_TRANS_DST:
        dma_rw(bps, false, &val, &bps->dma.dst, false);
        break;
    case REG_TRANS_CNT:
        dma_rw(bps, false, &val, &bps->dma.cnt, false);
        break;
    case REG_TRANS_CMD:
        dma_rw(bps, false, &val, &bps->dma.cmd, false);
        break;
    }

    return val;
}

static void BiscuitOS_mmio_write(void *opaque, hwaddr addr, uint64_t val,
                unsigned size)
{
    BiscuitOS_DMA_State *bps = opaque;

    if (addr < 0x80 && size != 4)
        return;

    if (addr >= 0x80 && size != 4 && size != 8)
        return;

    switch (addr) {
    case REG_DMA_STATUS:
        if (val & BISCUITOS_STATUS_IRQFACT)
            atomic_or(&bps->status, BISCUITOS_STATUS_IRQFACT);
        else
            atomic_and(&bps->status, ~BISCUITOS_STATUS_IRQFACT);
        break;
    case REG_INT_RAISE:
        BiscuitOS_raise_irq(bps, val);
        break;
    case REG_INT_DOWN:
        BiscuitOS_lower_irq(bps, val);
        break;
    case REG_TRANS_SRC:
        dma_rw(bps, true, &val, &bps->dma.src, false);
        break;
    case REG_TRANS_DST:
        dma_rw(bps, true, &val, &bps->dma.dst, false);
        break;
    case REG_TRANS_CNT:
        dma_rw(bps, true, &val, &bps->dma.cnt, false);
        break;
    case REG_TRANS_RUN:
        if (!(val & BISCUITOS_DMA_RUN))
            break;
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
    BiscuitOS_DMA_State *bps = opaque;

    while (1) {
        uint32_t val, ret = 1;

        qemu_mutex_lock(&bps->thr_mutex);
        while ((atomic_read(&bps->status) &
                   BISCUITOS_STATUS_COMPUTING) == 0 && !bps->stopping)
            qemu_cond_wait(&bps->thr_cond, &bps->thr_mutex);

        if (bps->stopping) {
            qemu_mutex_unlock(&bps->thr_mutex);
            break;
        }

        val = bps->fact;
        qemu_mutex_unlock(&bps->thr_mutex);

        while (val > 0)
            ret *= val--;

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
    BiscuitOS_DMA_State *bps = BISCUITOS(pdev);
    uint8_t *pci_conf = pdev->config;

    pci_config_set_interrupt_pin(pci_conf, 1);

    if (msi_init(pdev, 0, 1, true, false, errp))
        return;

    timer_init_ms(&bps->dma_timer,
                  QEMU_CLOCK_VIRTUAL, BiscuitOS_dma_timer, bps);

    qemu_mutex_init(&bps->thr_mutex);
    qemu_cond_init(&bps->thr_cond);
    qemu_thread_create(&bps->thread, "BiscuitOS-DMA", BiscuitOS_pci_thread,
                       bps, QEMU_THREAD_JOINABLE);

    memory_region_init_io(&bps->mmio, OBJECT(bps), &BiscuitOS_mmio_ops, bps,
                    "BiscuitOS-DMA-mmio", 1 * MiB);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &bps->mmio);

    /* update BiscuitOS DMA Firmware */
    sprintf(bps->dma_buf + BISCUITOS_FW_OFFSET, "%s", BISCUITOS_FW);
}

static void BiscuitOS_pci_uninit(PCIDevice *pdev)
{
    BiscuitOS_DMA_State *bps = BISCUITOS(pdev);

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
    BiscuitOS_DMA_State *bps = BISCUITOS(obj);

    bps->dma_mask = (1UL << 32) - 1;
    object_property_add(obj, "dma_mask", "uint64", BiscuitOS_obj_uint64,
                    BiscuitOS_obj_uint64, NULL, &bps->dma_mask, NULL);
}

static void BiscuitOS_class_init(ObjectClass *class, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(class);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(class);

    k->realize   = BiscuitOS_pci_realize;
    k->exit      = BiscuitOS_pci_uninit;
    k->vendor_id = BISCUITOS_DMA_VENDOR_ID;
    k->device_id = BISCUITOS_DMA_DEVICE_ID;
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
        .instance_size = sizeof(BiscuitOS_DMA_State),
        .instance_init = BiscuitOS_instance_init,
        .class_init    = BiscuitOS_class_init,
        .interfaces = interfaces,
    };

    type_register_static(&BiscuitOS_pci_info);
}
type_init(BiscuitOS_pci_register_types)
